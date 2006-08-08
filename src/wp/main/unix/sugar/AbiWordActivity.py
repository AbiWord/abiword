
import logging
import os
import time
import gtk

from sugar.activity.Activity import Activity


class AbiWord(gtk.Socket):

	def __init__ (self):
		gtk.Socket.__init__ (self)

		self.connect ('realize', self.realize_cb)


	def realize_cb (self, event):

		params = [
			'abiword', 
			'--nosplash', 
			'--gtk-socket-id=' + str (self.get_id ())
		]
		os.spawnvp (os.P_NOWAIT, 'abiword', params)


class AbiWordActivity (Activity):

	def __init__ (self, service, args):
		Activity.__init__ (self, service)
	
		self.set_title ("AbiWord")

		abiword = AbiWord ()
		self.add (abiword)
		abiword.show_all ()
