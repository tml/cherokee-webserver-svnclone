# -*- coding: utf-8 -*-
#
# Cherokee-admin
#
# Authors:
#      Alvaro Lopez Ortega <alvaro@alobbs.com>
#
# Copyright (C) 2001-2010 Alvaro Lopez Ortega
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of version 2 of the GNU General Public
# License as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#

import CTK
import Page
import Cherokee
import SelectionPanel
import validations

from CTK.Tab       import HEADER as Tab_HEADER
from CTK.Submitter import HEADER as Submit_HEADER
from CTK.TextField import HEADER as TextField_HEADER
from CTK.SortableList import HEADER as SortableList_HEADER

from util import *
from consts import *
from CTK.util import *
from CTK.consts import *
from configured import *


URL_BASE  = r'/vserver'
URL_APPLY = r'/vserver/apply'

HELPS = [('config_virtual_servers', N_("Virtual Servers"))]

NOTE_DELETE_DIALOG = N_('<p>You are about to delete the <b>%s</b> Virtual Server.</p><p>Are you sure you want to proceed?</p>')
NOTE_CLONE_DIALOG  = N_('You are about to clone a Virtual Server. Would you like to proceed?')
NOTE_NEW_NICK      = N_('Name of the Virtual Server you are about to create. A domain name is alright.')
NOTE_NEW_DROOT     = N_('Document Root directory of the new Virtual Server.')

VALIDATIONS = [
    ('tmp!new_droot', validations.is_dev_null_or_local_dir_exists),
    ('tmp!new_nick',  validations.is_new_vserver_nick)
]

JS_ACTIVATE_LAST = """
$('.selection-panel:first').data('selectionpanel').select_last();
"""

JS_CLONE = """
  var panel = $('.selection-panel:first').data('selectionpanel').get_selected();
  var url   = panel.find('.row_content').attr('url');
  $.ajax ({type: 'GET', async: false, url: url+'/clone', success: function(data) {
      // A transaction took place
      $('.panel-buttons').trigger ('submit_success');
  }});
"""

JS_PARTICULAR = """
  var vserver = window.location.pathname.match (/^\/vserver\/(\d+)/)[1];
  $.cookie ('%(cookie_name)s', vserver, { path: '/vserver' });
  window.location.replace ('%(url_base)s');
"""

def Commit():
    # New Virtual Server
    new_nick  = CTK.post.pop('tmp!new_nick')
    new_droot = CTK.post.pop('tmp!new_droot')
    if new_nick and new_droot:
        next = CTK.cfg.get_next_entry_prefix ('vserver')
        CTK.cfg['%s!nick'                   %(next)] = new_nick
        CTK.cfg['%s!document_root'          %(next)] = new_droot
        CTK.cfg['%s!rule!1!match'           %(next)] = 'default'
        CTK.cfg['%s!rule!1!handler'         %(next)] = 'common'

        CTK.cfg['%s!rule!2!match'           %(next)] = 'directory'
        CTK.cfg['%s!rule!2!match!directory' %(next)] = '/icons'
        CTK.cfg['%s!rule!2!handler'         %(next)] = 'file'
        CTK.cfg['%s!rule!2!document_root'   %(next)] = CHEROKEE_ICONSDIR

        CTK.cfg['%s!rule!3!match'           %(next)] = 'directory'
        CTK.cfg['%s!rule!3!match!directory' %(next)] = '/cherokee_themes'
        CTK.cfg['%s!rule!3!handler'         %(next)] = 'file'
        CTK.cfg['%s!rule!3!document_root'   %(next)] = CHEROKEE_THEMEDIR

        return {'ret': 'ok'}

    # Modifications
    return CTK.cfg_apply_post()


def reorder (arg):
    # Process new list
    order = CTK.post.pop(arg)
    tmp = order.split(',')
    tmp.reverse()

    # Build and alternative tree
    num = 10
    for v in tmp:
        CTK.cfg.clone ('vserver!%s'%(v), 'tmp!vserver!%d'%(num))
        num += 10

    # Set the new list in place
    del (CTK.cfg['vserver'])
    CTK.cfg.rename ('tmp!vserver', 'vserver')
    return {'ret': 'ok'}


class VirtualServerNew (CTK.Container):
    def __init__ (self):
        CTK.Container.__init__ (self)

        table = CTK.PropsTable()
        table.Add (_('Nick'),          CTK.TextCfg ('tmp!new_nick',  False, {'class': 'noauto'}), _(NOTE_NEW_NICK))
        table.Add (_('Document Root'), CTK.TextCfg ('tmp!new_droot', False, {'class': 'noauto'}), _(NOTE_NEW_DROOT))

        submit = CTK.Submitter (URL_APPLY)
        submit += table
        self += submit


class Render():
    class PanelList (CTK.Container):
        def __init__ (self, refresh, right_box):
            CTK.Container.__init__ (self)

            # Sanity check
            if not CTK.cfg.keys('vserver'):
                CTK.cfg['vserver!1!nick']           = 'default'
                CTK.cfg['vserver!1!document_root']  = '/tmp'
                CTK.cfg['vserver!1!rule!1!match']   = 'default'
                CTK.cfg['vserver!1!rule!1!handler'] = 'common'

            # Helper
            entry = lambda klass, key: CTK.Box ({'class': klass}, CTK.RawHTML (CTK.cfg.get_val(key, '')))

            # Build the panel list
            panel = SelectionPanel.SelectionPanel (reorder, right_box.id, URL_BASE, '')
            self += panel

            # Build the Virtual Server list
            vservers = CTK.cfg.keys('vserver')
            vservers.sort (lambda x,y: cmp(int(x), int(y)))
            vservers.reverse()

            for k in vservers:

                if k == vservers[-1]:
                    content = [entry('nick',  'vserver!%s!nick'%(k)),
                               entry('droot', 'vserver!%s!document_root'%(k))]
                    panel.Add (k, '/vserver/content/%s'%(k), content, draggable=False)
                else:
                    nick = CTK.cfg.get_val ('vserver!%s!nick'%(k), _('Unknown'))

                    # Remove
                    dialog = CTK.Dialog ({'title': _('Do you really want to remove it?'), 'width': 480})
                    dialog.AddButton (_('Remove'), CTK.JS.Ajax (URL_APPLY, async=False,
                                                                data    = {'vserver!%s'%(k):''},
                                                                success = dialog.JS_to_close() + \
                                                                    refresh.JS_to_refresh()))
                    dialog.AddButton (_('Cancel'), "close")
                    dialog += CTK.RawHTML (_(NOTE_DELETE_DIALOG) %(nick))
                    self += dialog
                    remove = CTK.ImageStock('del', {'class': 'del', 'title': _('Delete')})
                    remove.bind ('click', dialog.JS_to_show() + "return false;")

                    # Disable
                    if CTK.cfg.get_val('vserver!%s!disabled'%(k), False):
                        disabled = CTK.ImageStock('off', {'class': 'toggle-activation', 'title': 'Activate'})
                    else:
                        disabled = CTK.ImageStock('on', {'class': 'toggle-activation', 'title': 'Deactivate'})
                    disabled.bind ('click', "alert('TODO: Activate/Deactivate'); return false;")

                    group = CTK.Box ({'class': 'sel-actions'}, [disabled, remove])
                    content = [group]

                    content += [entry('nick',  'vserver!%s!nick'%(k)),
                                entry('droot', 'vserver!%s!document_root'%(k))]

                    # List entry
                    panel.Add (k, '/vserver/content/%s'%(k), content)

    class PanelButtons (CTK.Box):
        def __init__ (self):
            CTK.Box.__init__ (self, {'class': 'panel-buttons'})

            # Add New
            dialog = CTK.Dialog ({'title': _('Add New Virtual Server'), 'width': 480})
            dialog.AddButton (_('Add'), dialog.JS_to_trigger('submit'))
            dialog.AddButton (_('Cancel'), "close")
            dialog += VirtualServerNew()

            button = CTK.Button(_('New'))
            button.bind ('click', dialog.JS_to_show())
            dialog.bind ('submit_success', dialog.JS_to_close())
            dialog.bind ('submit_success', self.JS_to_trigger('submit_success'));

            self += button
            self += dialog

            # Clone
            dialog = CTK.Dialog ({'title': _('Clone Virtual Server'), 'width': 480})
            dialog.AddButton (_('Clone'), JS_CLONE + dialog.JS_to_close())
            dialog.AddButton (_('Cancel'), "close")
            dialog += CTK.RawHTML ('<p>%s</p>' %(_(NOTE_CLONE_DIALOG)))

            button = CTK.Button(_('Clone'))
            button.bind ('click', dialog.JS_to_show())

            self += dialog
            self += button

    def __call__ (self):
        title = _('Virtual Servers')

        # Content
        right = CTK.Box({'class': 'vserver_content'})
        left  = CTK.Box({'class': 'panel'}, CTK.RawHTML('<h2>%s</h2>'%(title) ))

        # Virtual Server List
        refresh = CTK.Refreshable ({'id': 'vservers_panel'})
        refresh.register (lambda: self.PanelList(refresh, right).Render())

        # Refresh on 'New' or 'Clone'
        buttons = self.PanelButtons()
        buttons.bind ('submit_success', refresh.JS_to_refresh (on_success=JS_ACTIVATE_LAST))

        left += refresh
        left += buttons

        # Refresh the list whenever the content change
        right.bind ('submit_success', refresh.JS_to_refresh());

        # Build the page
        headers = Tab_HEADER + Submit_HEADER + TextField_HEADER + SortableList_HEADER + SelectionPanel.HEADER
        page = Page.Base(title, body_id='vservers', helps=HELPS, headers=headers)
        page += left
        page += right

        return page.Render()


class RenderParticular():
    def __call__ (self):
        headers = SelectionPanel.HEADER
        page    = CTK.Page(headers=headers)

        props = {'url_base':    URL_BASE,
                 'cookie_name': SelectionPanel.COOKIE_NAME_DEFAULT}
        page += CTK.RawHTML (js=JS_PARTICULAR %(props))

        return page.Render()


CTK.publish (r'^%s$'    %(URL_BASE),  Render)
CTK.publish (r'^%s/\d+$'%(URL_BASE),  RenderParticular)
CTK.publish (r'^%s$'    %(URL_APPLY), Commit, method="POST", validation=VALIDATIONS)
