import os
import time
from base import *

DOCUMENTATION = """
   The Hypertext Transfer Protocol (HTTP) is an application-level
   protocol for distributed, collaborative, hypermedia information
   systems. It is a generic, stateless, protocol which can be used for
   many tasks beyond its use for hypertext, such as name servers and
   distributed object management systems, through extension of its
   request methods, error codes and headers [47]. A feature of HTTP is
   the typing and negotiation of data representation, allowing systems
   to be built independently of the data being transferred.
"""

class Test (TestBase):
    def __init__ (self):
        TestBase.__init__ (self)
        self.name = "If-Range header, 206 error"

        self.conf              = "Directory /if_range2 { Handler file }"
        self.expected_error    = 206
        self.forbidden_content = DOCUMENTATION

    def Prepare (self, www):
        d = self.Mkdir (www, "if_range2")
        f = self.WriteFile (d, "file", 0444, DOCUMENTATION)

        st = os.stat (f)
        mode, ino, dev, nlink, uid, gid, size, atime, mtime, ctime = st
        times = time.strftime("%a, %d %b %Y %H:%M:%S GMT", time.gmtime(mtime))
        
        self.request          = "GET /if_range2/file HTTP/1.1\r\n"  + \
                                "Host: localhost\r\n"               + \
                                "If-Range: %s\r\n" % (times)   + \
                                "Range: bytes=%d-\r\n" % (len(DOCUMENTATION)) 
