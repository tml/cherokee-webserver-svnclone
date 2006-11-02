import os
from base import *

DIR   = "/SCGI1/"
MAGIC = "Cherokee and SCGI rocks!"
PORT  = 5001

SCRIPT = """
from scgi.scgi_server import *

class TestHandler (SCGIHandler):
    def handle_connection (self, socket):
        s = socket.makefile ('w')
        s.write('Content-Type: text/plain\\r\\n\\r\\n')
        s.write('%s\\n')
        socket.close()
        
SCGIServer(TestHandler, port=%d).serve()
""" % (MAGIC, PORT)

CONF = """
vserver!default!directory!%s!handler = scgi
vserver!default!directory!%s!handler!balancer = round_robin
vserver!default!directory!%s!handler!balancer!type = interpreter
vserver!default!directory!%s!handler!balancer!local_scgi1!host = localhost:%d
vserver!default!directory!%s!handler!balancer!local_scgi1!interpreter = %s %s
vserver!default!directory!%s!priority = 1260
"""

class Test (TestBase):
    def __init__ (self):
        TestBase.__init__ (self)
        self.name = "SCGI I"

        self.request           = "GET %s HTTP/1.0\r\n" %(DIR)
        self.expected_error    = 200
        self.expected_content  = MAGIC
        self.forbidden_content = ["scgi.scgi_server", "SCGIServer", "write"]

    def Prepare (self, www):
        scgi_file = self.WriteFile (www, "scgi_test1.scgi", 0444, SCRIPT)

        self.conf = CONF % (DIR, DIR, DIR, DIR, PORT, DIR, PYTHON_PATH, scgi_file, DIR)

    def Precondition (self):
        re = os.system ("%s -c 'import scgi.scgi_server' 2>/dev/null" % (PYTHON_PATH)) 
        return (re == 0)
