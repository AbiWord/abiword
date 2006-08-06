
import logging
import os
import time
import gtk

from sugar.activity.Activity import Activity


class AbiWord(gtk.Socket):

	def __init__ (self):
		gtk.Socket.__init__ (self)


	def run (self):

		os.spawnvp (os.P_NOWAIT, 'abiword', ['abiword', '--gtk-socket-id=' + str (self.get_id ())])


class AbiWordActivity (Activity):

	def __init__ (self, service, args):
		Activity.__init__ (self, service)
	
		self.set_title ("AbiWord")

		abiword = AbiWord ()
		abiword.run ()

		abiword.show_all ()
