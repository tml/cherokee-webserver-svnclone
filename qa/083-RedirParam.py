from base import *

CONF = """
vserver!default!rule!directory!/redirparam!handler = redir
vserver!default!rule!directory!/redirparam!priority = 830
vserver!default!rule!directory!/redirparam!handler!rewrite!1!show = 0
vserver!default!rule!directory!/redirparam!handler!rewrite!1!regex = ^/([0-9]+)/(.+)$
vserver!default!rule!directory!/redirparam!handler!rewrite!1!substring = /redirparam/file.php?arg1=$1&arg2=$2
"""

class Test (TestBase):
    def __init__ (self):
        TestBase.__init__ (self)
        self.name             = "Redir Param"
        self.request          = "GET /redirparam/123/cosa HTTP/1.0\r\n"
        self.expected_error   = 200
        self.expected_content = "one=123 two=cosa"
        self.conf             = CONF

    def Prepare (self, www):
        self.Mkdir (www, "redirparam")
        self.WriteFile (www, "redirparam/file.php", 0444,
                        '<?php echo "one=".$_GET["arg1"]." two=".$_GET["arg2"]?>')

    def Precondition (self):
        return os.path.exists (look_for_php())

