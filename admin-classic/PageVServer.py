import validations
import util

from Page import *
from Form import *
from Table import *
from Entry import *
from consts import *
from Rule import *
from RuleList import *
from CherokeeManagement import *
from Wizard import *
from GraphManager import *

# For gettext
N_ = lambda x: x

DATA_VALIDATION = [
    ("vserver!.*?!user_dir",                   validations.is_safe_id),
    ("vserver!.*?!document_root",             (validations.is_dev_null_or_local_dir_exists, 'cfg')),
    ("vserver!.*?!post_max_len",              (validations.is_positive_int)),
    ("vserver!.*?!ssl_certificate_file",      (validations.is_local_file_exists, 'cfg', 'nochroot')),
    ("vserver!.*?!ssl_certificate_key_file",  (validations.is_local_file_exists, 'cfg', 'nochroot')),
    ("vserver!.*?!ssl_ca_list_file",          (validations.is_local_file_exists, 'cfg', 'nochroot')),
    ("vserver!.*?!ssl_verify_depth",          (validations.is_positive_int)),
    ("vserver!.*?!logger!.*?!filename",       (validations.parent_is_dir, 'cfg', 'nochroot')),
    ("vserver!.*?!logger!.*?!command",        (validations.is_local_file_exists, 'cfg')),
    ("vserver!.*?!logger!x_real_ip_access$",   validations.is_ip_or_netmask_list),
]

DEFAULT_LOGGER_TEMPLATE = '${ip_remote} - ${user_remote} ${now} ${request_original} ${status}'

RULE_LIST_NOTE = N_("<p>Rules are evaluated from <b>top to bottom</b>. Drag & drop them to reorder.</p>")

DEFAULT_HOST_NOTE = N_("""
<p>The 'default' virtual server matches all the domain names.</p>
""")

NOTE_NICKNAME         = N_('Nickname for the virtual server.')
NOTE_CERT             = N_('This directive points to the PEM-encoded Certificate file for the server (Full path to the file)')
NOTE_CERT_KEY         = N_('PEM-encoded Private Key file for the server (Full path to the file)')
NOTE_CA_LIST          = N_('File containing the trusted CA certificates, utilized for checking the client certificates (Full path to the file)')
NOTE_CIPHERS          = N_('Ciphers that TLS/SSL is allowed to use. <a target="_blank" href="http://www.openssl.org/docs/apps/ciphers.html">Reference</a>. (Default: all ciphers supported by the OpenSSL version used).')
NOTE_CLIENT_CERTS     = N_('Skip, Accept or Require client certificates.')
NOTE_VERIFY_DEPTH     = N_('Limit up to which depth certificates in a chain are used during the verification procedure (Default: 1)')
NOTE_ERROR_HANDLER    = N_('Allows the selection of how to generate the error responses.')
NOTE_PERSONAL_WEB     = N_('Directory inside the user home directory to use as root web directory. Disabled if empty.')
NOTE_DISABLE_PW       = N_('The personal web support is currently turned on.')
NOTE_ADD_DOMAIN       = N_('Adds a new domain name. Wildcards are allowed in the domain name.')
NOTE_DOCUMENT_ROOT    = N_('Virtual Server root directory.')
NOTE_DIRECTORY_INDEX  = N_('List of name files that will be used as directory index. Eg: <em>index.html,index.php</em>.')
NOTE_MAX_UPLOAD_SIZE  = N_('The maximum size, in bytes, for POST uploads. (Default: unlimited)')
NOTE_KEEPALIVE        = N_('Whether this virtual server is allowed to use Keep-alive (Default: yes)')
NOTE_DISABLE_LOG      = N_('The Logging is currently enabled.')
NOTE_LOGGERS          = N_('Logging format. Apache compatible is highly recommended here.')
NOTE_ACCESSES         = N_('Back-end used to store the log accesses.')
NOTE_ERRORS           = N_('Back-end used to store the log errors.')
NOTE_ACCESSES_ERRORS  = N_('Back-end used to store the log accesses and errors.')
NOTE_WRT_FILE         = N_('Full path to the file where the information will be saved.')
NOTE_WRT_EXEC         = N_('Path to the executable that will be invoked on each log entry.')
NOTE_X_REAL_IP        = N_('Whether the logger should read and use the X-Real-IP and X-Forwarded-For headers (send by reverse proxies).')
NOTE_X_REAL_IP_ALL    = N_('Accept all the X-Real-IP and X-Forwarded-For headers. WARNING: Turn it on only if you are centain of what you are doing.')
NOTE_X_REAL_IP_ACCESS = N_('List of IP addresses and subnets that are allowed to send X-Real-IP and X-Forwarded-For headers.')
NOTE_EVHOST           = N_('How to support the "Advanced Virtual Hosting" mechanism. (Default: off)')
NOTE_LOGGER_TEMPLATE  = N_('The following variables are accepted: <br/>${ip_remote}, ${ip_local}, ${protocol}, ${transport}, ${port_server}, ${query_string}, ${request_first_line}, ${status}, ${now}, ${time_secs}, ${time_nsecs}, ${user_remote}, ${request}, ${request_original}, ${vserver_name}, ${response_size}')
NOTE_MATCHING_METHOD  = N_('Allows the selection of domain matching method.')
NOTE_COLLECTOR        = N_('Whether or not it should collected statistics about the traffic of this virtual server.')
NOTE_UTC_TIME         = N_('Time standard to use in the log file entries.')

HELPS = [
    ('config_virtual_servers', N_("Virtual Servers")),
    ('modules_loggers',        N_("Loggers")),
    ('cookbook_ssl',           N_("SSL cookbook"))
]

RULE_NAME_LEN_LIMIT = 35

class PageVServer (PageMenu, FormHelper):
    def __init__ (self, cfg):
        PageMenu.__init__ (self, 'vserver', cfg, HELPS)
        FormHelper.__init__ (self, 'vserver', cfg)

        self._priorities         = None
        self._priorities_userdir = None
        self._rule_table         = 1

    def _op_handler (self, uri, post):
        assert (len(uri) > 1)

        host = uri.split('/')[1]
        self.set_submit_url ('/vserver/%s'%(host))
        self.submit_ajax_url = "/vserver/%s/ajax_update"%(host)

        # Check whether host exists
        cfg = self._cfg['vserver!%s'%(host)]
        if not cfg:
            return '/vserver/'
        if not cfg.has_child():
            return '/vserver/'

        default_render = False
        if '/wizard/' in uri:
            re = self._op_apply_wizard (host, uri, post)
            if re: return re

        elif post.get_val('is_submit'):
            if (post.get_val('tmp!new_rule!value') or
                post.get_val('tmp!new_rule!bypass_value_check')):
                re = self._op_add_new_entry (post       = post,
                                             cfg_prefix = 'vserver!%s!rule' %(host),
                                             url_prefix = '/vserver/%s'%(host),
                                             key_prefix = 'tmp!new_rule')
                if not self.has_errors() and re:
                    return re

            elif post.get_val('tmp!new_rule!user_dir!value'):
                re = self._op_add_new_entry (post       = post,
                                             cfg_prefix = 'vserver!%s!user_dir!rule' %(host),
                                             url_prefix = '/vserver/%s/userdir'%(host),
                                             key_prefix = 'tmp!new_rule!user_dir')
                if not self.has_errors() and re:
                    return re

            elif post.get_val('tmp!clone_rule'):
                re = self._op_clone_rule (post)
                if not self.has_errors() and re:
                    return re

            else:
                # It's updating properties
                re = self._op_apply_changes (host, uri, post)
                if re: return re

        elif uri.endswith('/ajax_update'):
            if post.get_val('update_prio'):
                cfg_key = post.get_val('update_prefix')
                rules = RuleList(self._cfg, cfg_key)

                changes = []
                for tmp in post['update_prio']:
                    changes.append(tmp.split(','))

                rules.change_prios (changes)
                return "ok"

            self.ApplyChangesDirectly (post)
            return 'ok'

        # Ensure the default rules are set
        if self._cfg.get_val('vserver!%s!user_dir'%(host)):
            tmp = self._cfg["vserver!%s!user_dir!rule"%(host)]
            if not tmp:
                pre = "vserver!%s!user_dir!rule!1" %(host)
                self._cfg["%s!match"       %(pre)] = "default"
                self._cfg["%s!handler"     %(pre)] = "common"
                self._cfg["%s!match!final" %(pre)] = "1"

        self._priorities         = RuleList(self._cfg, 'vserver!%s!rule'%(host))
        self._priorities_userdir = RuleList(self._cfg, 'vserver!%s!user_dir!rule'%(host))

        return self._op_render_vserver_details (host)

    def _op_add_new_entry (self, post, cfg_prefix, url_prefix, key_prefix):
        # Build the configuration prefix
        rules = RuleList(self._cfg, cfg_prefix)
        priority = rules.get_highest_priority() + 100
        pre = '%s!%d' % (cfg_prefix, priority)

        # Look for the rule type
        _type = post.get_val (key_prefix)

        # Build validation list
        validation = DATA_VALIDATION[:]

        # The 'add_new_entry' checking function depends on
        # the whether 'add_new_type' is a directory, an extension
        # or a regular extension
        rule_module = module_obj_factory (_type, self._cfg, "%s!match"%(pre), self.submit_url)
        if 'validation' in dir(rule_module):
            validation += rule_module.validation

        # Validate
        self._ValidateChanges (post, validation)
        if self.has_errors():
            return

        # Read the properties
        filtered_post = {}
        for p in post:
            if not p.startswith(key_prefix):
                continue

            prop = p[len('%s!'%(key_prefix)):]
            if not prop: continue

            filtered_post[prop] = post[p][0]

        # Apply the changes to the configuration tree
        self._cfg['%s!match'%(pre)] = _type
        rule_module.apply_cfg (filtered_post)

        del(self._cfg['tmp!new_rule'])

        # Get to the details page
        return "%s/rule/%d" % (url_prefix, priority)

    def _op_render_vserver_details (self, host):
        content = self._render_vserver_guts (host)
        nick = self._cfg.get_val ('vserver!%s!nick'%(host))

        self.AddMacroContent ('title', '%s: %s' %(_('Virtual Server'), nick))
        self.AddMacroContent ('content', content)

        return Page.Render(self)

    def _render_vserver_guts (self, host):
        pre = "vserver!%s" % (host)
        cfg = self._cfg[pre]
        name = self._cfg.get_val ('vserver!%s!nick'%(host))

        tabs = []
        txt = "<h1>%s: %s</h1>" % (_('Virtual Server'), name)

        # Basics
        tmp = self._render_basics(host)
        tabs += [(_('Basics'), tmp)]

        # Domains
        tmp = self._render_hosts(host, name)
        tabs += [('Host Match', tmp)]

        # Behavior
        pre = 'vserver!%s!rule' %(host)
        tmp = self.Dialog(_(RULE_LIST_NOTE))
        tmp += '<div class="rulesdiv">'
        tmp += self._render_rules_generic (cfg_key    = pre,
                                          url_prefix = '/vserver/%s'%(host),
                                          priorities = self._priorities)

        tmp += '<div class="rulessection" id="newsection">'
        tmp += self._render_add_rule ("tmp!new_rule")
        tmp += '</div>'

        tmp += '<div class="rulessection" id="clonesection">'
        tmp += self._render_clone_rule (pre)
        tmp += '</div>'

        tmp += '<div class="rulessection" id="wizardsection">'
        tmp += self._render_wizards (host)
        tmp += '</div>'

        tmp += '<div class="rulesbutton"><a id="newsection_b">%s</a></div>' % (_('Add new rule'))
        tmp += '<div class="rulesbutton"><a id="clonesection_b">%s</a></div>' % (_('Clone rule'))
        tmp += '<div class="rulesbutton"><a id="wizardsection_b">%s</a></div>' % (_('Wizards'))

        tmp += '</div>\n'
        tabs += [(_('Behavior'), tmp)]

        # Personal Webs
        tmp  = self._render_personal_webs (host)
        if self._cfg.get_val('vserver!%s!user_dir'%(host)):
            tmp += "<br/><hr />"
            pre = 'vserver!%s!user_dir!rule'%(host)
            tmp += self._render_rules_generic (cfg_key    = pre,
                                               url_prefix = '/vserver/%s/userdir'%(host),
                                               priorities = self._priorities_userdir)
            tmp += self._render_add_rule ("tmp!new_rule!user_dir")
        tabs += [(_('Personal Webs'), tmp)]

        # Error handlers
        tmp = self._render_error_handler(host)
        tabs += [(_('Error handler'), tmp)]

        # Logging
        tmp  = self._render_logger(host)
        tmp += self._render_graphs(host)
        tabs += [(_('Logging'), tmp)]

        # Security
        tmp = self._render_ssl(host)
        tabs += [(_('Security'), tmp)]

        txt += self.InstanceTab (tabs)
        form = Form (self.submit_url, add_submit=False)
        return form.Render(txt)

    def _render_ssl (self, host):
        pre = 'vserver!%s' % (host)

        txt = '<h2>%s</h2>' % (_('Required SSL/TLS values'))
        table = TableProps()
        self.AddPropEntry (table, _('Certificate'),      '%s!ssl_certificate_file' % (pre),       _(NOTE_CERT), optional=True)
        self.AddPropEntry (table, _('Certificate key'),  '%s!ssl_certificate_key_file' % (pre),   _(NOTE_CERT_KEY), optional=True)
        txt += self.Indent(table)

        txt += '<h2>%s</h2>' % (_('Advanced options'))
        table = TableProps()
        self.AddPropEntry (table, _('Ciphers'), '%s!ssl_ciphers' % (pre), _(NOTE_CIPHERS), optional=True)
        self.AddPropOptions_Ajax (table, _('Client Certs. Request'),
                                         '%s!ssl_client_certs' % (pre),
                                         CLIENT_CERTS,
                                         _(NOTE_CLIENT_CERTS))

        req_cc = self._cfg.get_val('%s!ssl_client_certs' % (pre))
        if req_cc:
            self.AddPropEntry (table, _('CA List'),     '%s!ssl_ca_list_file' % (pre),        _(NOTE_CA_LIST))

            calist = self._cfg.get_val('%s!ssl_ca_list_file' % (pre))
            if calist:
                self.AddPropEntry (table, _('Verify Depth'),  '%s!ssl_verify_depth' % (pre),  _(NOTE_VERIFY_DEPTH), size=4)

        txt += self.Indent(table)

        return txt

    def _render_error_handler (self, host):
        pre = 'vserver!%s' % (host)

        txt = '<h2>%s</h2>' % (_('Error Handling hook'))
        table = TableProps()
        e = self.AddPropOptions_Reload_Module (table, _('Error Handler'),
                                               '%s!error_handler' % (pre),
                                               modules_available(ERROR_HANDLERS),
                                               NOTE_ERROR_HANDLER)
        txt += self.Indent(table) + e
        return txt

    def _render_add_rule (self, prefix):
        # Render
        txt = "<h2>%s</h2>" % (_('Add new rule'))
        table = TableProps()
        e = self.AddPropOptions_Reload_Module (table, _("Rule Type"), prefix,
                                               modules_available(RULES), "")
        txt += self.Indent (str(table) + e)
        return txt

    def _get_handler_name (self, mod_name):
        for h in HANDLERS:
            if h[0] == mod_name:
                return _(h[1])

    def _get_auth_name (self, mod_name):
        for h in VALIDATORS:
            if h[0] == mod_name:
                return _(h[1])

    def _render_rules_generic (self, cfg_key, url_prefix, priorities):
        txt = ''

        if not len(priorities):
            return txt


        table_name = "rules%d" % (self._rule_table)
        self._rule_table += 1

        ENABLED_IMAGE   = self.InstanceImage('tick.png',  _('Yes'))
        DISABLED_IMAGE  = self.InstanceImage('cross.png', _('No'))
        FORBIDDEN_IMAGE = self.InstanceImage('forbidden.png', _('Forbidden'))

        txt += '<table id="%s" class="rulestable">' % (table_name)
        txt += '<tr class="nodrag nodrop"><th>&nbsp;</th><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th></th></tr>' %(
            _('Target'), _('Type'), _('Handler'), _('Root'), _('Auth'), _('Enc'), _('Exp'), _('Time'), _('Final'), _('Active'))

        # Rule list
        for prio in priorities:
            conf = priorities[prio]

            _type = conf.get_val('match')
            pre   = '%s!%s!match' % (cfg_key, prio)

            # Try to load the rule plugin
            rule = Rule(self._cfg, pre, self.submit_url, self.errors, 0)
            name      = rule.get_name()
            name_type = rule.get_type_name()

            # Ensure the name is too long
            if len(name) > RULE_NAME_LEN_LIMIT:
                name = "%s<b>...</b>" % (name[:RULE_NAME_LEN_LIMIT])

            if _type != 'default':
                link      = '<a href="%s/rule/%s">%s</a>' % (url_prefix, prio, name)
                disabled  = self.InstanceCheckbox ('%s!%s!disabled'%(cfg_key, prio), self._cfg.get_val('%s!%s!disabled'%(cfg_key, prio)) == 1, quiet=True, switch=True, noautosubmit=True)
                final     = self.InstanceCheckbox ('%s!final'%(pre), True, quiet=True)
                link_del  = self.AddDeleteLink (self.submit_ajax_url, "%s!%s"%(cfg_key, prio))
                if self._cfg.get_val('%s!%s!disabled'%(cfg_key, prio)) == "1":
                    extra = ' class="trdisabled"'
                else:
                    extra     = ''
                draggable = ' class="dragHandle"'
            else:
                link      = '<a href="%s/rule/%s">%s</a>' % (url_prefix, prio, _('Default'))
                disabled  = self.HiddenInput ('%s!%s!disabled'%(cfg_key, prio), "0")
                final     = self.HiddenInput ('%s!final'%(pre), "1")
                link_del  = ''
                extra     = ' class="nodrag nodrop"'
                draggable = ''

            if conf.get_val('handler'):
                handler_name = self._get_handler_name (conf['handler'].value)
            else:
                handler_name = DISABLED_IMAGE

            if conf.get_val('auth'):
                auth_name = self._get_auth_name (conf['auth'].value)
            else:
                auth_name = DISABLED_IMAGE

            expiration    = [DISABLED_IMAGE, ENABLED_IMAGE]['expiration' in conf.keys()]
            document_root = [DISABLED_IMAGE, ENABLED_IMAGE]['document_root' in conf.keys()]
            timeout       = [DISABLED_IMAGE, ENABLED_IMAGE]['timeout' in conf.keys()]

            encoders = DISABLED_IMAGE
            if 'encoder' in conf.keys():
                for k in conf['encoder'].keys():
                    val = conf.get_val('encoder!%s'%(k))
                    if val in ['1', 'allow']:
                        encoders = ENABLED_IMAGE
                    elif val == 'forbid':
                        encoders = FORBIDDEN_IMAGE

            txt += '<!-- %s --><tr prio="%s" id="%s"%s><td%s>&nbsp;</td><td>%s</td><td>%s</td><td>%s</td><td class="center">%s</td><td class="center">%s</td><td class="center">%s</td><td class="center">%s</td><td class="center">%s</td><td class="center">%s</td><td class="center switch">%s</td><td class="center">%s</td></tr>\n' % (
                prio, pre, prio, extra, draggable, link, name_type, handler_name, document_root, auth_name, encoders, expiration, timeout, final, disabled, link_del)

        txt += '</table>\n'

        txt += '''
               <script type="text/javascript">
                   $(document).ready(function(){
                      init_PageVServer ("%(name)s", "%(url)s", "%(prefix)s");
                   });
               </script>
               ''' % {'name':   table_name,
                      'url' :   self.submit_ajax_url,
                      'prefix': cfg_key}
        return txt

    def _render_wizards (self, host):
        txt = ''
        pre = 'vserver!%s'%(host)

        mgr = WizardManager (self._cfg, "Rules", pre)
        txt += mgr.render ("/vserver/%s"%(host))

        table = '<table id="wizSel" class="rulestable"><tr><th>Category</th><th>Wizard</th></tr>'
        table += '<tr><td id="wizG"></td><td id="wizL"></td></table>'

        if txt:
            txt = _("<h2>Wizards</h2>") + table + txt

        return txt

    def _render_personal_webs (self, host):
        txt = '<h2>%s</h2>' % (_('/~user directories'))

        table = TableProps()
        cfg_key = 'vserver!%s!user_dir'%(host)
        if self._cfg.get_val(cfg_key):
            js = "post_del_key('%s','%s');" % (self.submit_ajax_url, cfg_key)
            button = self.InstanceButton (_("Disable"), onClick=js)
            self.AddProp (table, _('Status'), '', button, _(NOTE_DISABLE_PW))

        self.AddPropEntry (table, _('Directory name'), cfg_key, _(NOTE_PERSONAL_WEB), optional=True)
        txt += self.Indent(table)

        return txt

    def _render_basics (self, host):
        pre = "vserver!%s" % (host)

        txt = ''
        if host != "default":
            txt += '<h2>%s</h2>' % (_('Server ID'))
            table = TableProps()
            self.AddPropEntry (table, _('Virtual Server nickname'), '%s!nick'%(pre), _(NOTE_NICKNAME))
            txt += self.Indent(table)

        txt += '<h2>%s</h2>' % (_('Paths'))
        table = TableProps()
        self.AddPropEntry (table, _('Document Root'),     '%s!document_root'%(pre),   _(NOTE_DOCUMENT_ROOT))
        self.AddPropEntry (table, _('Directory Indexes'), '%s!directory_index'%(pre), _(NOTE_DIRECTORY_INDEX))
        txt += self.Indent(table)

        txt += '<h2>%s</h2>' % (_('Network'))
        table = TableProps()
        self.AddPropCheck (table, _('Keep-alive'),         '%s!keepalive'%(pre), True, _(NOTE_KEEPALIVE))
        self.AddPropEntry (table, _('Max Upload Size'),    '%s!post_max_len' % (pre),  _(NOTE_MAX_UPLOAD_SIZE), optional=True)
        txt += self.Indent(table)

        txt += '<h2>%s</h2>' % (_('Advanced Virtual Hosting'))
        table = TableProps()
        e = self.AddPropOptions_Reload_Module (table, _('Method'), '%s!evhost'%(pre),
                                               modules_available(EVHOSTS), _(NOTE_EVHOST))
        txt += self.Indent(table) + e

        return txt

    def _render_graphs (self, host):
        txt  = ''
        pre  = "vserver!%s" % (host)
        name = self._cfg.get_val('vserver!%s!nick'%(host), "default")

        # Server collects statistics
        srv_has = (self._cfg.get_val('server!collector') != None)

        # VServer options
        txt += '<h2>%s</h2>' % (_('Statistics'))
        table = TableProps()
        if srv_has:
            self.AddPropCheck (table, _('Collect Statistics'), '%s!collector!enabled'%(pre), True, _(NOTE_COLLECTOR))
        else:
            self.AddPropCheck (table, _('Collect Statistics'), '%s!collector!enabled'%(pre), True, _(NOTE_COLLECTOR), disabled=1)

        txt += self.Indent(table)
        txt += "<br/>"

        # VServer collect statistics
        if not srv_has:
            return txt

        if not int(self._cfg.get_val('%s!collector!enabled'%(pre), '1')):
            return txt

        if not graphs_are_active(self._cfg):
            return txt

        txt += '<script type="text/javascript" src="/static/js/graphs.js"></script>';
        txt += '<div id="grapharea">'
        txt += ' <div id="gtype">'
        txt += '  <div id="g1m" class="gbutton"><a onclick="graphChangeInterval(\'1m\')">%s</a></div>' % (_("1 month"))
        txt += '  <div id="g1w" class="gbutton"><a onclick="graphChangeInterval(\'1w\')">%s</a></div>' % (_("1 week"))
        txt += '  <div id="g1d" class="gbutton"><a onclick="graphChangeInterval(\'1d\')">%s</a></div>' % (_("1 day"))
        txt += '  <div id="g6h" class="gbutton"><a onclick="graphChangeInterval(\'6h\')">%s</a></div>' % (_("6 hours"))
        txt += '  <div id="g1h" class="gbutton gsel"><a onclick="graphChangeInterval(\'1h\')">%s</a></div>' % (_("1 hour"))
        txt += ' </div>'
        txt += ' <div id="graphdiv">'
        txt += '  <img id="graphimg" src="/graphs/vserver_traffic_%s_1h.png" alt="Graph" />' % (name)
        txt += ' </div>'
        txt += '</div>'

        txt += """<script type="text/javascript">
                   graphPrefix   = "vserver";
                   graphType     = "traffic";
                   graphVServer  = "%s";
                   graphInterval = "1h";
                   $(document).ready(function() { refreshGraph(); });
                  </script>
               """ % (name)

        return txt

    def _render_logger (self, host):
        txt = ""

        # Error logging
        txt += "<h2>%s</h2>" %(_("Error logging"))

        pre = 'vserver!%s!error_writer'%(host)
        format = self._cfg.get_val(pre)

        writers = LOGGER_WRITERS[:]
        writers.insert (0, ('', N_('Disabled')))

        cfg_key = "%s!type"%(pre)
        table = TableProps()
        self.AddPropOptions_Ajax (table, _('Write errors to'), cfg_key, writers, _(NOTE_ERRORS))

        error = self._cfg.get_val(cfg_key)
        if error == 'file':
            self.AddPropEntry (table, _('Filename'), '%s!filename'%(pre), _(NOTE_WRT_FILE))
        elif error == 'exec':
            self.AddPropEntry (table, _('Command'), '%s!command'%(pre), _(NOTE_WRT_EXEC))

        txt += self.Indent(table)

        # Access logging
        txt += "<h2>%s</h2>" %(_("Access logging"))
        tmp  = ""

        pre = 'vserver!%s!logger'%(host)
        format = self._cfg.get_val(pre)

        table = TableProps()
        self.AddPropOptions_Ajax (table, _('Format'), pre,
                                  modules_available(LOGGERS), _(NOTE_LOGGERS))

        if format:
            cfg_key = "%s!access!type"%(pre)
            self.AddPropOptions_Ajax (table, _('Accesses'), cfg_key, LOGGER_WRITERS, _(NOTE_ACCESSES))

            access = self._cfg.get_val(cfg_key)
            if not access or access == 'file':
                self.AddPropEntry (table, _('Filename'), '%s!access!filename'%(pre), _(NOTE_WRT_FILE))
            elif access == 'exec':
                self.AddPropEntry (table, _('Command'), '%s!access!command'%(pre), _(NOTE_WRT_EXEC))

            if format == 'custom':
                self._add_logger_template(table, pre, 'access')

        txt += self.Indent(table)

        if format:
            # Options
            table = TableProps()

            x_real_ip     = int(self._cfg.get_val('%s!x_real_ip_enabled'%(pre), "0"))
            x_real_ip_all = int(self._cfg.get_val('%s!x_real_ip_access_all'%(pre), "0"))

            self.AddPropOptions (table, _('Time standard'), '%s!utc_time'%(pre), UTC_TIME, _(NOTE_UTC_TIME))
            self.AddPropCheck (table, _('Accept Forwarded IPs'), '%s!x_real_ip_enabled'%(pre), False, _(NOTE_X_REAL_IP))
            if x_real_ip:
                self.AddPropCheck (table, _('Don\'t check origin'), '%s!x_real_ip_access_all'%(pre), False, _(NOTE_X_REAL_IP_ALL))
                if not x_real_ip_all:
                    self.AddPropEntry (table, _('Accept from Hosts'), '%s!x_real_ip_access'%(pre), _(NOTE_X_REAL_IP_ACCESS))

            txt += "<h3>%s</h3>" %(_("Logging Option"))
            txt += self.Indent(table)

        return txt

    def _render_hosts (self, host, name):
        pre = "vserver!%s" % (host)
        txt = '<h2>%s</h2>' % (_('Host names'))

        if name == 'default':
            txt += self.Dialog(DEFAULT_HOST_NOTE)
            return txt

        table = TableProps()
        e = self.AddPropOptions_Reload_Module (table, _('Matching method'),
                                               '%s!match' % (pre),
                                               modules_available(VRULES),
                                               _(NOTE_MATCHING_METHOD))
        txt += self.Indent(table) + e
        return txt

    def _op_apply_wizard (self, host, uri, post):
        tmp  = uri.split('/')
        name = tmp[3]

        mgr = WizardManager (self._cfg, "Rules", 'vserver!%s'%(host))
        wizard = mgr.load (name)

        output = wizard.run ("/vserver%s"%(uri), post)
        if output:
            return output

        return '/vserver/%s' % (host)

    def _op_apply_changes (self, host, uri, post):
        pre = "vserver!%s" % (host)

        # Error handler
        self.ApplyChanges_OptionModule ('%s!error_handler'%(pre), uri, post)

        # EVHost
        self.ApplyChanges_OptionModule ('%s!evhost'%(pre), uri, post)

        # EVHost
        self.ApplyChanges_OptionModule ('%s!match'%(pre), uri, post)

        # Look for the checkboxes
        checkboxes = ['%s!keepalive'%(pre),
                      '%s!collect_statistics'%(pre),
                      '%s!logger!x_real_ip_enabled'%(pre),
                      '%s!logger!x_real_ip_access_all'%(pre),
                      '%s!collector!enabled'%(pre)]

        tmp = self._cfg['%s!rule'%(pre)]
        if tmp and tmp.has_child():
            for p in tmp:
                checkboxes.append('%s!rule!%s!disabled'%(pre,p))
                checkboxes.append('%s!rule!%s!match!final'%(pre,p))

        # Special case for loggers
        if not post.get_val('%s!logger'%pre):
            del (self._cfg['%s!logger'%(pre)])
        else:
            keys = self._cfg.keys('%s!logger'%pre)
            for key in keys:
                cfg_key = '%s!logger!%s!filename' % (pre,key)
                self.Validate_NotEmpty (post, cfg_key, _("Filename must be set"))

        # Apply changes
        self.ApplyChanges (checkboxes, post, DATA_VALIDATION)

        # Clean up: Error Log
        cfg_key = 'vserver!%s!error_writer'%(host)
        self._cleanup_writer_cfg (cfg_key)

        # Clean up: Access Log
        cfg_key = 'vserver!%s!logger!access'%(host)
        self._cleanup_writer_cfg (cfg_key)


    def _cleanup_writer_cfg (self, pre):
        logger = self._cfg.get_val('%s!type'%(pre))
        if not logger:
            del (self._cfg[pre])
            return

        to_be_deleted = []
        to_be_deleted.append('%s!all' %(pre))

        for entry in self._cfg[pre]:
            if entry == 'type':
                continue

            if logger == "stderr" or \
               logger == "syslog":
                to_be_deleted.append(entry)
            elif logger == "file" and \
                 entry != "filename":
                to_be_deleted.append(entry)
            elif logger == "exec" and \
                 entry != "command":
                to_be_deleted.append(entry)

        for entry in to_be_deleted:
            key = "%s!%s" % (pre, entry)
            del(self._cfg[key])


    def _add_logger_template (self, table, prefix, target):
        cfg_key = '%s!%s_template' % (prefix, target)
        value = self._cfg.get_val(cfg_key)
        if not value:
            self._cfg[cfg_key] = DEFAULT_LOGGER_TEMPLATE
        self.AddPropEntry (table, _('Template: '), cfg_key, _(NOTE_LOGGER_TEMPLATE))


    def _render_clone_rule (self, prefix):
        # Render
        txt = "<h2>%s</h2>" % (_('Clone rule'))
        table = Table(2, 1, header_style='width="200px"')
        table += (_('Rule'),)
        fo1 = Form ("/vserver", add_submit=False, auto=False)

        clonable = [("", _("Choose a rule.."))]
        rule_list = self._cfg[prefix].keys()
        rule_list.sort(reverse=True)

        for r in rule_list[:-1]:
            pre   = '%s!%s!match' % (prefix, r)
            # Try to load the rule plugin
            rule = Rule(self._cfg, pre, self.submit_url, self.errors, 0)
            name      = rule.get_name()
            clonable.append(("%s!%s"%(prefix, r), name))

        op1 = self.InstanceOptions ("tmp!clone_rule", clonable, noautosubmit=True)
        table += (op1[0], SUBMIT_CLONE)

        txt += self.Indent(fo1.Render(str(table)))
        return txt

    def _op_clone_rule (self, post):
       rule_source = post.pop('tmp!clone_rule')
       tmp         = rule_source.split('!')
       vserver     = '!'.join(tmp[:-2])
       next_rule   = util.cfg_vsrv_rule_get_next (self._cfg, vserver)
       rule_target = next_rule[1]
       error = self._cfg.clone(rule_source, rule_target)
       if not error:
           return '/%s'%(rule_target.replace('!','/'))