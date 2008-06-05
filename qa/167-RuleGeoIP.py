from base import *

MAGIC = "This QA tests the GeoIP rule module"
DIR   = "GeoIP1_nomatch"
FILE  = "test"

CONF = """
vserver!default!rule!1670!match = directory
vserver!default!rule!1670!match!directory = /%s
vserver!default!rule!1670!handler = cgi

vserver!default!rule!1671!match = and
vserver!default!rule!1671!match!left = geoip
vserver!default!rule!1671!match!left!countries = ES,US,UK,CA
vserver!default!rule!1671!match!right = directory
vserver!default!rule!1671!match!right!directory = /%s
vserver!default!rule!1671!handler = file
"""

CGI = """#!/bin/sh

echo "Content-Type: text/plain"
echo 
echo "%s"
""" % (MAGIC)

class Test (TestBase):
    def __init__ (self):
        TestBase.__init__ (self)
        self.name = "GeoIP: no match"

        self.request           = "GET /%s/%s HTTP/1.0\r\n" % (DIR, FILE)
        self.expected_error    = 200
        self.expected_content  = MAGIC
        self.forbidden_content = ["/bin/sh", "echo"]
        self.conf              = CONF % (DIR, DIR)

    def Precondition (self):
        return True
        # Check that pam module was compiled
        if not self.has_module("geoip"):
            return False
        return True

    def Prepare (self, www):
        d = self.Mkdir (www, DIR)
        f = self.WriteFile (d, FILE, 0755, CGI)
