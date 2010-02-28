# Cheroke Admin: RRD plug-in
#
# Authors:
#      Alvaro Lopez Ortega <alvaro@alobbs.com>
#
# Copyright (C) 2009-2010 Alvaro Lopez Ortega
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

from Rule import RulePlugin
from util import *

URL_APPLY = '/plugin/directory/apply'

NOTE_DIRECTORY = N_("Public Web Directory to which content the configuration will be applied.")

def apply():
    # POST info
    key      = CTK.post.pop ('key', None)
    vsrv_num = CTK.post.pop ('vsrv_num', None)
    new_dir  = CTK.post.pop ('tmp!directory', None)

    # New
    if new_dir:
        next_rule, next_pre = cfg_vsrv_rule_get_next ('vserver!%s'%(vsrv_num))

        CTK.cfg['%s!match'%(next_pre)]            = 'directory'
        CTK.cfg['%s!match!directory'%(next_pre)] = new_dir

        return {'ret': 'ok', 'redirect': '/vserver/%s/rule/%s' %(vsrv_num, next_rule)}

    # Modifications
    for k in CTK.post:
        CTK.cfg[k] = CTK.post[k]
    return {'ret': 'ok'}


class Plugin_directory (RulePlugin):
    def __init__ (self, key, **kwargs):
        RulePlugin.__init__ (self, key)

        table = CTK.PropsTable()
        table.Add (_('Web Directory'), CTK.TextCfg('%s!directory'%(key)), _(NOTE_DIRECTORY))

        submit = CTK.Submitter (URL_APPLY)
        submit += CTK.Hidden ('key', key)
        submit += CTK.Hidden ('vsrv_num', kwargs.pop('vsrv_num', ''))
        submit += table
        self += submit

        # Validation, and Public URLs
        CTK.publish (URL_APPLY, apply, method="POST")

    def GetName (self):
        directory = CTK.cfg.get_val ('%s!directory' %(self.key), '')
        return "Directory %s" %(directory)
