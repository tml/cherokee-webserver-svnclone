import validations

from Page import *
from Form import *
from Table import *
from Entry import *
from consts import *
from VirtualServer import *

DATA_VALIDATION = [
    ("vserver!.*?!document_root",                  validations.is_local_dir_exists),
    ("vserver!.*?!ssl_certificate_file",           validations.is_local_file_exists),
    ("vserver!.*?!ssl_certificate_key_file",       validations.is_local_file_exists),
    ("vserver!.*?!ssl_ca_list_file",               validations.is_local_file_exists),
    ("vserver!.*?!logger!access!filename",         validations.parent_is_dir),
    ("vserver!.*?!logger!error!filename",          validations.parent_is_dir),
    ("vserver!.*?!logger!access!command",          validations.is_local_file_exists),
    ("vserver!.*?!logger!error!command",           validations.is_local_file_exists),
    ("vserver!.*?!rule!default!priority",          validations.is_positive_int),
    ("vserver!.*?!rule!.*?!.*?!priority",          validations.is_positive_int),
    ("vserver!.*?!user_dir!rule!default!priority", validations.is_positive_int),
    ("vserver!.*?!user_dir!rule!.*?!.*?!priority", validations.is_positive_int)
]

RULE_LIST_NOTE = """
<p>When a request is handled, the server tries to match a rule <b>from
bottom to top</b>. The first match is applied.</p>
"""

NOTE_CERT            = 'This directive points to the PEM-encoded Certificate file for the server.'
NOTE_CERT_KEY        = 'PEM-encoded Private Key file for the server.'
NOTE_CA_LIST         = 'File with the certificates of Certification Authorities (CA) whose clients you deal with.'
NOTE_ERROR_HANDLER   = 'Allow to select how to generate the error responses.'
NOTE_PERSONAL_WEB    = 'Directory inside the user home directory that will be used as the root web directory.'
NOTE_DISABLE_PW      = 'The personal web support is currently turned on.'
NOTE_ADD_DOMAIN      = 'Adds a new domain name. Wildcards are allowed in the domain name.'
NOTE_DOCUMENT_ROOT   = 'Virtual Server root directory.'
NOTE_DIRECTORY_INDEX = 'List of name files that will be used as directory index. Eg: <em>index.html,index.php</em>.'
NOTE_LOGGERS         = 'Logging format. Apache compatible is highly recommended here.'
NOTE_ACCESSES        = 'Back-end with which the accesses will be stored.'
NOTE_ERRORS          = 'Back-end with which the errors that may occur will be stored.'
NOTE_WRT_FILE        = 'Full path to the file were the information will be saved.'
NOTE_WRT_EXEC        = 'Path to the executable that will be invoked with each log entry.'


class PageVServer (PageMenu, FormHelper):
    def __init__ (self, cfg):
        PageMenu.__init__ (self, 'vserver', cfg)
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

        if post.get_val('is_submit'):
            if post.get_val('add_new_entry'):
                # It's adding a new entry
                re = self._op_add_new_entry (post       = post,
                                             cfg_prefix = 'vserver!%s' %(host),
                                             url_prefix = '/vserver/%s'%(host))
                if not self.has_errors() and re:
                    return re
            elif post.get_val('userdir_add_new_entry'):
                # It's adding a new user entry 
                re = self._op_add_new_entry (post       = post,
                                             cfg_prefix = 'vserver!%s!user_dir'%(host),
                                             url_prefix = '/vserver/%s/userdir'%(host),
                                             key_prefix = 'userdir_')
                if not self.has_errors() and re:
                    return re
            else:
                # It's updating properties
                self._op_apply_changes (host, uri, post)

        elif uri.endswith('/ajax_update'):
            self._op_apply_changes (host, uri, post)
            return 'ok'

        # Ensure the default rules are there
        if not self._cfg['vserver!%s!rule!default'%(host)].has_child():
            self._cfg["vserver!%s!rule!default!handler" %(host)] = "common"
            self._cfg["vserver!%s!rule!default!priority"%(host)] = "1"

        if self._cfg.get_val('vserver!%s!user_dir'%(host)) and \
                not self._cfg['vserver!%s!user_dir!rule!default'%(host)]:
            self._cfg["vserver!%s!user_dir!rule!default!handler" %(host)] = "common"
            self._cfg["vserver!%s!user_dir!rule!default!priority"%(host)] = "1"

        self._priorities         = VServerEntries (host, self._cfg)
        self._priorities_userdir = VServerEntries (host, self._cfg, user_dir=True)

        return self._op_render_vserver_details (host)

    def _op_add_new_entry (self, post, cfg_prefix, url_prefix, key_prefix=''):
        key_add_new_type     = key_prefix + 'add_new_type'
        key_add_new_entry    = key_prefix + 'add_new_entry'
        key_add_new_handler  = key_prefix + 'add_new_handler'

        # The 'add_new_entry' checking function depends on 
        # the whether 'add_new_type' is a directory, an extension
        # or a regular extension
        validation = DATA_VALIDATION[:]

        type_ = post.get_val(key_add_new_type)
        if type_ == 'directory':
            validation += [(key_add_new_entry, validations.is_dir_formated)]
        elif type_ == 'extensions':
            validation += [(key_add_new_entry, validations.is_safe_id_list)]
        elif type_ == 'request':
            validation += [(key_add_new_entry, validations.is_regex)]

        # Apply changes
        self._ValidateChanges (post, validation)
        if self.has_errors():
            return

        entry    = post.pop(key_add_new_entry)
        type_    = post.pop(key_add_new_type)
        handler  = post.pop(key_add_new_handler)

        # Look for the highest priority on the list
        prio_max = 1
        for c in self._cfg[cfg_prefix]:
            for d in self._cfg["%s!%s"%(cfg_prefix,c)]:
                pre = "%s!rule!%s!%s!priority"%(cfg_prefix,c,d)
                tmp = self._cfg.get_val(pre)
                if tmp:
                    if int(tmp) > prio_max:
                        prio_max = int(tmp)
        priority = str(prio_max + 100)

        pre = "%s!rule!%s!%s" % (cfg_prefix, type_, entry)
        self._cfg["%s!handler"%(pre)]  = handler
        self._cfg["%s!priority"%(pre)] = priority

        return "%s/prio/%s" % (url_prefix, priority)

    def _op_render_vserver_details (self, host):
        content = self._render_vserver_guts (host)

        self.AddMacroContent ('title', 'Virtual Server: %s' %(host))
        self.AddMacroContent ('content', content)

        return Page.Render(self)

    def _render_vserver_guts (self, host):
        pre = "vserver!%s" % (host)
        cfg = self._cfg[pre]
        
        tabs = []
        txt = "<h1>Virtual Server: %s</h1>" % (host)

        # Basics
        table = TableProps()
        self.AddPropEntry (table, 'Document Root',     '%s!document_root'%(pre),   NOTE_DOCUMENT_ROOT)
        self.AddPropEntry (table, 'Directory Indexes', '%s!directory_index'%(pre), NOTE_DIRECTORY_INDEX)
        tabs += [('Basics', str(table))]

        # Domains
        tmp = self._render_hosts(host)
        tabs += [('Domain names', tmp)]
        
        # Behaviour
        tmp = self._render_rules_generic (cfg_key    = 'vserver!%s' %(host), 
                                          url_prefix = '/vserver/%s'%(host),
                                          priorities = self._priorities)
        tmp += self._render_add_rule(host)
        tabs += [('Behaviour', tmp)]

        # Personal Webs
        tmp  = self._render_personal_webs (host)
        if self._cfg.get_val('vserver!%s!user_dir'%(host)):
            tmp += "<p><hr /></p>"
            tmp += self._render_rules_generic (cfg_key    = 'vserver!%s!user_dir'%(host), 
                                               url_prefix = '/vserver/%s/userdir'%(host),
                                               priorities = self._priorities_userdir)
            tmp += self._render_add_rule (host, prefix="userdir_")
        tabs += [('Personal Webs', tmp)]

        # Error handlers
        tmp = self._render_error_handler(host)
        tabs += [('Error handler', tmp)]        

        # Logging
        tmp = self._render_logger(host)
        tabs += [('Logging', tmp)]

        # Security
        table = TableProps()
        self.AddPropEntry (table, 'Certificate',     '%s!ssl_certificate_file' % (pre),     NOTE_CERT)
        self.AddPropEntry (table, 'Certificate key', '%s!ssl_certificate_key_file' % (pre), NOTE_CERT_KEY)
        self.AddPropEntry (table, 'CA List',         '%s!ssl_ca_list_file' % (pre),         NOTE_CA_LIST)
        tabs += [('Security', str(table))]

        txt += self.InstanceTab (tabs)

        form = Form (self.submit_url)
        return form.Render(txt)

    def _render_error_handler (self, host):
        txt = ''
        pre = 'vserver!%s' % (host)
        
        table = TableProps()
        e = self.AddPropOptions_Reload (table, 'Error Handler',
                                        '%s!error_handler' % (pre), 
                                        ERROR_HANDLERS, NOTE_ERROR_HANDLER)
        txt += str(table) + self.Indent(e)

        return txt

    def _render_add_rule (self, host, prefix=''):
        # Add new rule
        txt      = ''
        entry    = self.InstanceEntry (prefix+'add_new_entry', 'text')
        type     = EntryOptions (prefix+'add_new_type',    ENTRY_TYPES, selected='directory')
        handler  = EntryOptions (prefix+'add_new_handler', HANDLERS,    selected='common')
        
        table  = Table(4,1)
        table += ('Type', 'Entry', 'Handler')
        table += (type, entry, handler)

        txt += "<h3>Add new rule</h3>"
        txt += str(table)
        return txt

    def _render_rules_generic (self, cfg_key, url_prefix, priorities):
        txt = ''
        txt += self.Dialog(RULE_LIST_NOTE)

        if len(priorities):
            table_name = "rules%d" % (self._rule_table)
            self._rule_table += 1

            txt += '<h3>Rule list</h3>'
            txt += '<table id="%s" class="rulestable">' % (table_name)
            txt += '<tr NoDrag="1" NoDrop="1"><th>Target</th><th>Type</th><th>Handler</th></tr>'
            
            # Rule list
            for rule in priorities:
                type, name, prio, conf = rule

                if type != 'default':
                    pre  = '%s!rule!%s!%s' % (cfg_key, type, name)
                    link = '<a href="%s/prio/%s">%s</a>' % (url_prefix, prio, name)
                else:
                    pre  = '%s!rule!%s' % (cfg_key, type)
                    link = '<a href="%s/prio/%s">Default</a>' % (url_prefix, prio)

                e1 = EntryOptions ('%s!handler' % (pre), HANDLERS, selected=conf['handler'].value)

                if type != 'default':
                    js = "post_del_key('%s', '%s');" % (self.submit_ajax_url, pre)
                    link_del = self.InstanceImage ("bin.png", "Delete", border="0", onClick=js)
                    extra = ''
                else:
                    extra = ' NoDrag="1" NoDrop="1"'
                    link_del = ''

                txt += '<!-- %s --><tr id="%s"%s><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>\n' % (
                    prio, pre, extra, link, type.capitalize(), e1, link_del)

            txt += '</table>\n'
            txt += '''<script type="text/javascript">
                      $(document).ready(function() {
                        $("#%(name)s tr:even').addClass('alt')");

                        $('#%(name)s').tableDnD({
                          onDrop: function(table, row) {
                              var rows = table.tBodies[0].rows;
                              var post = '';
                              for (var i=1; i<rows.length; i++) {
                                var prio = (i > 0) ? i*100 : 1;
                                post += rows[i].id + '!priority=' + prio + '&';
                              }
	                      jQuery.post ('%(url)s', post, 
                                  function (data, textStatus) {
                                      window.location.reload();  
                                  }
                              );
                          }
                        });
                      });
                      </script>
                   ''' % {'name': table_name, 
                          'url' : self.submit_ajax_url}
        return txt

    def _render_personal_webs (self, host):
        txt = ''

        table = TableProps()
        cfg_key = 'vserver!%s!user_dir'%(host)
        if self._cfg.get_val(cfg_key):
            js = "post_del_key('%s','%s');" % (self.submit_ajax_url, cfg_key)
            button = self.InstanceButton ("Disable", onClick=js)
            self.AddProp (table, 'Status', '', button, NOTE_DISABLE_PW)

        self.AddPropEntry (table, 'Directory name', cfg_key, NOTE_PERSONAL_WEB)
        txt += str(table)

        return txt

    def _render_logger (self, host):
        pre = 'vserver!%s!logger'%(host)

        # Logger
        table = TableProps()
        self.AddPropOptions_Ajax (table, 'Format', pre, LOGGERS, NOTE_LOGGERS)

        txt  = '<h3>Logging Format</h3>'
        txt += self.Indent(str(table))
        
        # Writers
        if self._cfg.get_val(pre):
            writers = ''

            # Accesses
            cfg_key = "%s!access!type"%(pre)
            table = TableProps()
            self.AddPropOptions_Ajax (table, 'Accesses', cfg_key, LOGGER_WRITERS, NOTE_ACCESSES)
            writers += str(table)

            access = self._cfg.get_val(cfg_key)
            if access == 'file':
                t1 = TableProps()
                self.AddPropEntry (t1, 'Filename', '%s!access!filename'%(pre), NOTE_WRT_FILE, size=40)
                writers += str(t1)
            elif access == 'exec':
                t1 = TableProps()
                self.AddPropEntry (t1, 'Command', '%s!access!command'%(pre), NOTE_WRT_EXEC, size=40)
                writers += str(t1)

            writers += "<hr />"

            # Error
            cfg_key = "%s!error!type"%(pre)
            table = TableProps()
            self.AddPropOptions_Ajax (table, 'Errors', cfg_key, LOGGER_WRITERS, NOTE_ERRORS)
            writers += str(table)

            error = self._cfg.get_val(cfg_key)
            if error == 'file':
                t1 = TableProps()
                self.AddPropEntry (t1, 'Filename', '%s!error!filename'%(pre), NOTE_WRT_FILE, size=40)
                writers += str(t1)
            elif error == 'exec':
                t1 = TableProps()
                self.AddPropEntry (t1, 'Command', '%s!error!command'%(pre), NOTE_WRT_EXEC, size=40)
                writers += str(t1)

            txt += '<h3>Writers</h3>'
            txt += self.Indent(writers)

        return txt

    def _render_hosts (self, host):
        cfg_domains = self._cfg["vserver!%s!domain"%(host)]

        txt       = ""
        available = "1"

        if cfg_domains and \
           cfg_domains.has_child():
            table = Table(2,1)
            table += ('Domain pattern', '')

            # Build list
            for i in cfg_domains:
                domain = cfg_domains[i].value
                cfg_key = "vserver!%s!domain!%s" % (host, i)
                en = self.InstanceEntry (cfg_key, 'text')
                js = "post_del_key('%s','%s');" % (self.submit_ajax_url, cfg_key)
                link_del = self.InstanceImage ("bin.png", "Delete", border="0", onClick=js)
                table += (en, link_del)

            txt += str(table)
            txt += "<hr />"

        # Look for firs available
        i = 1
        while cfg_domains:
            if not cfg_domains[str(i)]:
                available = str(i)
                break
            i += 1

        # Add new domain
        table = TableProps()
        cfg_key = "vserver!%s!domain!%s" % (host, available)
        self.AddPropEntry (table, 'Add new domain name', cfg_key, NOTE_ADD_DOMAIN)
        txt += str(table)

        return txt

    def _op_apply_changes (self, host, uri, post):
        pre = "vserver!%s" % (host)

        # Error handler
        self.ApplyChanges_OptionModule ('%s!error_handler'%(pre), uri, post)

        # Apply changes
        self.ApplyChanges ([], post, DATA_VALIDATION)

        # Clean old logger properties
        self._cleanup_logger_cfg (host)

    def _cleanup_logger_cfg (self, host):
        cfg_key = "vserver!%s!logger" % (host)
        logger = self._cfg.get_val (cfg_key)
        if not logger:
            del(self._cfg[cfg_key])
            return

        to_be_deleted = []
        for entry in self._cfg[cfg_key]:
            if logger == "stderr" or \
               logger == "syslog":
                to_be_deleted.append(cfg_key)
            elif logger == "file" and \
                 entry != "filename":
                to_be_deleted.append(cfg_key)
            elif logger == "exec" and \
                 entry != "command":
                to_be_deleted.append(cfg_key)

        for entry in to_be_deleted:
            key = "%s!%s" % (cfg_key, entry)
            del(self._cfg[key])
