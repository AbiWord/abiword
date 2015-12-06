import sys
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
from ..overrides import override
from ..importer import modules

Abi = modules['Abi']._introspection_module

# Initialize libabiword
Abi.init(sys.argv)
