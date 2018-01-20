#!/usr/bin/python
# -*- coding: utf-8 -*-
################################################################################
# Copyright 1998-2018 by authors (see AUTHORS.txt)
#
#   This file is part of LuxCoreRender.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

import os
import time
import logging
import socket
import threading
import functools 

import pyluxcore
import pyluxcoretools.utils.loghandler as loghandler
import pyluxcoretools.utils.socket as socketutils
import pyluxcoretools.utils.md5 as md5utils
import pyluxcoretools.renderfarm.renderfarmfilmmerger as filmmerger
import pyluxcoretools.renderfarm.renderfarm as renderfarm

logger = logging.getLogger(loghandler.loggerName + ".renderfarm")

class RenderFarmJobSingleImage:
	def __init__(self, renderFarm, renderConfigFileName):
		self.lock = threading.RLock()
		self.renderFarm = renderFarm
		self.nodeThreads = list()

		logging.info("New render farm job: " + renderConfigFileName)
		self.renderConfigFileName = renderConfigFileName
		# Compute the MD5 of the renderConfigFile
		self.renderConfigFileMD5 = md5utils.md5sum(renderConfigFileName)
		logging.info("Job file md5: " + self.renderConfigFileMD5)

		baseName = os.path.splitext(renderConfigFileName)[0]
		self.filmFileName = baseName + ".flm"
		self.imageFileName = baseName + ".png"
		self.workDirectory = baseName + "-netrendering"
		self.seed = 1

		# Check the work directory 
		if (not os.path.exists(self.workDirectory)):
			# Create the work directory
			os.makedirs(self.workDirectory)
		elif (not os.path.isdir(self.workDirectory)):
			raise ValueError("Can not use " + self.workDirectory + " as work directory")

		self.filmHaltSPP = 0
		self.filmHaltTime = 0
#		self.filmHaltConvThreshold = 3.0 / 256.0

	def SetFilmHaltSPP(self, v):
		with self.lock:
			self.filmHaltSPP = v
	def GetFilmHaltSPP(self):
		with self.lock:
			return self.filmHaltSPP

	def SetFilmHaltTime(self, v):
		with self.lock:
			self.filmHaltTime = v
	def GetFilmHaltTime(self):
		with self.lock:
			return self.filmHaltTime

#	def SetFilmHaltConvThreshold(self, v):
#		with self.lock:
#			self.filmHaltConvThreshold = v
#	def GetFilmHaltConvThreshold(self, v):
#		with self.lock:
#			return self.filmHaltConvThreshold

	def GetSeed(self):
		with self.lock:
			seed = self.seed
			self.seed += 1
			return self.seed

	def GetRenderConfigFileName(self):
		with self.lock:
			return self.renderConfigFileName

	def GetFilmFileName(self):
		with self.lock:
			return self.filmFileName

	def GetImageFileName(self):
		with self.lock:
			return self.imageFileName

	def GetWorkDirectory(self):
		with self.lock:
			return self.workDirectory

	def GetStartTime(self):
		with self.lock:
			return self.jobStartTime

	def GetNodeThreadsList(self):
		with self.lock:
			return list(self.nodeThreads)

	def Start(self):
		with self.lock:
			self.jobStartTime = time.time()
			
			# Put all nodes at work
			nodesList = list(self.renderFarm.nodes.values())
			for node in nodesList:
				# Put the new node at work
				NewNode(node)
				nodeThread.Start()
			
			# Start the film merger
			self.filmMerger = filmmerger.RenderFarmFilmMerger(self)
			self.filmMerger.Start()

	def Stop(self, stopFilmMerger = True, lastUpdate = False):
		with self.lock:
			logging.info("Job done: " + self.renderConfigFileName)
		
			if stopFilmMerger:
				# Stop the film merger
				self.filmMerger.Stop()
			
			if lastUpdate:
				# Tell the node threads to do an update
				for nodeThread in self.nodeThreads:
					nodeThread.UpdateFilm()
				self.filmMerger.MergeAllFilms()

			# Tell the threads to stop
			for nodeThread in self.nodeThreads:
				logging.info("Waiting for ending of: " + nodeThread.thread.name)
				nodeThread.Stop()

	def NewNode(self, node):
		with self.lock:
			if node.state != renderfarm.NodeState.FREE:
				return

			# Put the new node at work
			nodeThread = RenderFarmJobSingleImageThread(self, node)
			self.nodeThreads.append(nodeThread)
			nodeThread.Start()

class RenderFarmJobSingleImageThread:
	def __init__(self, jobSingleImage, renderFarmNode):
		self.lock = threading.RLock()
		self.renderFarmNode = renderFarmNode
		self.jobSingleImage = jobSingleImage
		self.thread = None
		# Get an unique seed
		self.seed = str(self.jobSingleImage.GetSeed())

		self.eventCondition = threading.Condition(self.lock)
		self.eventStop = False
		self.eventUpdateFilm = False
		
		self.workDirectory = self.jobSingleImage.GetWorkDirectory()

	def GetNodeFilmFileName(self):
		return self.workDirectory + "/" + self.thread.name.replace(":", "_") + "-" + self.seed + ".flm"

	def Start(self):
		with self.lock:
			try:
				self.renderFarmNode.state = renderfarm.NodeState.RENDERING
				
				key = self.renderFarmNode.GetKey()
				self.thread = threading.Thread(target=functools.partial(RenderFarmJobSingleImageThread.NodeThread, self))
				self.thread.name = "RenderFarmNodeThread-" + key
				
				self.thread.start()
			except:
				self.renderFarmNode.state = renderfarm.NodeState.ERROR
				logging.exception("Error while initializing")

	def UpdateFilm(self):
		with self.eventCondition:
			if (not self.thread.is_alive):
				return

			self.eventUpdateFilm = True
			self.eventCondition.notify()

	def Stop(self):
		with self.eventCondition:
			if (not self.thread.is_alive):
				return

			self.eventStop = True
			self.eventCondition.notify()

		self.thread.join()
		
		self.renderFarmNode.state = renderfarm.NodeState.FREE

	def NodeThread(self):
		logger.info("Node thread started")

		# Connect with the node
		nodeSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		try:
			nodeSocket.connect((self.renderFarmNode.address, self.renderFarmNode.port))

			#-------------------------------------------------------------------
			# Send the LuxCore version (they must match)
			#-------------------------------------------------------------------

			socketutils.SendLine(nodeSocket, pyluxcore.Version())
			socketutils.RecvOk(nodeSocket)
			logging.info("Remote node has the same pyluxcore verison")

			#-------------------------------------------------------------------
			# Send the RenderConfig serialized file
			#-------------------------------------------------------------------

			socketutils.SendFile(nodeSocket, self.jobSingleImage.GetRenderConfigFileName())

			#-------------------------------------------------------------------
			# Send the seed
			#-------------------------------------------------------------------

			logging.info("Sending seed: " + self.seed)
			socketutils.SendLine(nodeSocket, self.seed)

			#-------------------------------------------------------------------
			# Receive the rendering start
			#-------------------------------------------------------------------

			logging.info("Waiting for node rendering start")
			result = socketutils.RecvLine(nodeSocket)
			if (result != "RENDERING_STARTED"):
				logging.info(result)
				raise RuntimeError("Error while waiting for the rendering start")
			logging.info("Node rendering started")

			#-------------------------------------------------------------------
			# Receive stats and film
			#-------------------------------------------------------------------

			lastFilmUpdate = time.time()
			with self.eventCondition:
				while True:
					timeTofilmUpdate = self.jobSingleImage.renderFarm.filmUpdatePeriod - (time.time() - lastFilmUpdate)

					if ((timeTofilmUpdate <= 0.0) or self.eventUpdateFilm):
						# Time to request a film update
						socketutils.SendLine(nodeSocket, "GET_FILM")
						# TODO add SafeSave
						socketutils.RecvFile(nodeSocket, self.GetNodeFilmFileName())
						lastFilmUpdate = time.time()
						self.eventUpdateFilm = False

					if (self.eventStop):
						logging.info("Waiting for node rendering stop")
						socketutils.SendLine(nodeSocket, "DONE")
						socketutils.RecvOk(nodeSocket)
						break

					# Print some rendering node statistic
					socketutils.SendLine(nodeSocket, "GET_STATS")
					result = socketutils.RecvLine(nodeSocket)
					if (result.startswith("ERROR")):
						logging.info(result)
						raise RuntimeError("Error while waiting for the rendering statistics")
					logger.info("Node rendering statistics: " + result)

					self.eventCondition.wait(min(timeTofilmUpdate, self.jobSingleImage.renderFarm.statsPeriod))
		except Exception as e:
			logging.exception(e)
			self.renderFarmNode.state = renderfarm.NodeState.ERROR
		else:
			self.renderFarmNode.state = renderfarm.NodeState.FREE
		finally:
				try:
					nodeSocket.shutdown(socket.SHUT_RDWR)
				except:
					pass

				try:
					nodeSocket.close()
				except:
					pass

		logger.info("Node thread done")
