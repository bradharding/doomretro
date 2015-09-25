/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

char *wilv[] =
{
    // 0 = 'A'
    "       jjjjjj        jj[YQQSaj      jb\\_\\abW[dj    ja\\Yajj_Y[ajû   j\\Y["
    "jjj_Y\\_jû  jaWWajUU[Y[ajû  j\\WYjj_b_Y\\ajû jaWUajjjj_[[_jû j[QSjûûûja[Yaj"
    "ûj\\WQajû  ja[Wajûj\\\\[jûû  jba_bjûjjjjjû   jjjjjjû ûûûûû    ûûûûûû",

    // 1 = 'B'
    "jjjjjjjjjjjjj   jYSQSUPPQQPW_j  j___ad____SSU_j jjjjjjjjjaSWS_jûjjjjjjjjja["
    "SPbjûj[QQQQPQQQWSbjûûj[[UY\\\\_\\\\WPbjû j[\\S[jjjjaSSP[j j[\\UYjjjjaQWY[jû"
    "j\\[WYPQPPQUWYbjûjba_a[\\YY\\[[_jûûjjjjjjjjjjjjjûû  ûûûûûûûûûûûûû  ",

    // 2 = 'C'
    "    jjjjjjjjjj   jjb\\\\YQQQQYjû jbYdddd\\a\\_ajû j\\_dfjjjjjjjjûjb\\_djûûû"
    "ûûûûûûj_\\adjû        j_\\abjû        jb\\bdjû         j\\bd\\jjjjjjjj  jdb"
    "dbUQUQSQYjû  jjgbb\\baaa_jû   ûjjjjjjjjjjû     ûûûûûûûûûû",

    // 3 = 'D'
    "jjjjjjjjjjj     h\\QQQQPPQUajj   jba___\\\\_YYWbj  jjjjjjjjj_WUYjû jjjjjjûû"
    "ûjWQYbj j\\WW\\jû  jWQU_jûj_[\\_jû  jWQS_jûj\\[[_jû  jUQSbjûj\\YY\\jjjjUUQ["
    "jûûj\\[WUQPPUWS[bjû ja__\\\\\\\\__ajjûû jjjjjjjjjjjûûû   ûûûûûûûûûûû    ",

    // 4 = 'E'
    "    jjjjjjjjjj   jj_UYSSUUS\\jû jd[YYabadd_bjû j_WW_jjjjjjjjûjaYWW_jjjjjjjj"
    "ûj_YWWWWUUUQQ_jûj_YYWYadbda_bjûjaWWU_jjjjjjjjû j[QS_jjjjjjjjû jd_SWSQQQSU[j"
    "û  jja\\__a_aaajû   ûjjjjjjjjjjû     ûûûûûûûûûû",

    // 5 = 'F'
    "    jjjjjjjj   jjaUPPPQ[jû jdaY___\\\\ajû j\\aWajjjjjjûjb\\\\Wjjjjûûûûj_[WU"
    "PPYjû   j_YUU_\\ajû   j\\[W_jjjjû   j_YS\\jûûûû   j_YQ\\jû      jaa[ajû    "
    "  jjjjjjû       ûûûûûû      ",

    // 6 = 'G'
    "    jjjjjjjjjjjj   jjb\\\\WUQQQQU_jû jd\\_dfdd\\a\\__bjû jY\\_djjjjjjjjjjûj"
    "b[\\_jûûûûûûûûûûûja\\\\ajû   jjjjjj ja\\\\ajû   jYPP\\jûjaW\\bjû   j\\QP_jû"
    " jS\\bdjjjjj_QP_jû jd[bd[QPQS\\QP_jû  jjfgbb\\baa[[ajû   ûjjjjjjjjjjjjû    "
    " ûûûûûûûûûûûû",

    // 7 = 'H'
    "jjjjjj   jjjjjj j\\UU[jû  j[QQ[jûja_\\\\jû  j\\YWajûja\\\\_jû  j\\WU_jûja\\"
    "\\_jjjjj\\WQ_jûja\\\\\\YUQQSYWQ[jûja[[aaa____WQ[jûj_Y[ajjjjj_UQ[jûj_WYajûûû"
    "j_UQ[jûj_Y[_jû  j\\SS[jûja__bjû  ja\\[ajûjhjjhjû  jjjjjjû ûûûûûû   ûûûûûû",

    // 8 = 'I'
    "jjjjjj j[SS\\jûj__\\bjûj\\a\\ajûj_aaajûj_a_ajûj_b_bjûj\\_bbjûj\\[_ajûj\\W\\"
    "_jûja_aajûjjjjjjû ûûûûûû",

    // 9 = 'J'
    "     jjjjjj      j[QUajû     jbY_djû     jbaabjû     jbabbjû     ja_\\bjû  "
    "   j_\\\\ajû     jb_Y_jûjjjjj_b\\Y_jûjYSUSQ__\\jjûj_\\__[bbjjûûjjjjjjjjûûû "
    " ûûûûûûûû   ",

    // 10 = 'K'
    "jjjjjj   jjjjjj  jYQPWjû jPPPP\\jû j[YSUjûjPUYWajûû jYSUYjjQUUU\\jûû  j[QP["
    "jQWUS_jûû   j_SQUWY[UUjûû    j\\SQ[j_USSQj     j[UQ[jj\\SQSWj    j\\WQYjûj"
    "\\QS\\Uj   jYWSYjû j[U\\[Wj  j_\\\\_jû  jbdabYj jjjjjjû   jjjjjjû ûûûûûû   "
    " ûûûûûû",

    // 11 = 'L'
    "jjjjjj      jaUQ[jû     jd_Ybjû     jbaabjû     jbbabjû     jb\\_ajû     ja"
    "\\\\_jû     j_Y_bjû     j_Y\\b_jjjjj  j\\__QSUSYjû jjbb[__\\_jû  ûjjjjjjjjû"
    "    ûûûûûûûû",

    // 12 = 'M'
    "  jjj      jjj    jWPYj    jUPSj  j\\QPWYj  jW\\UY\\j j[SSYb[jjS_[Y\\bjûj\\"
    "SW\\bbUUW[W[\\bjûjYSWYdba_WYU\\adjûjYSU[dba\\YWSaaejûjYUS\\jdaY_j[abbjûjWUU"
    "[jja_jj\\aabjûjWSS[jûjjûj\\b__jûj\\\\\\_jû ûûjadbajûjjjjjjû   jjjjjjû ûûûûû"
    "û    ûûûûûû",

    // 13 = 'N'
    "  jjj     jjjjjj  jPPaj    j\\¨P\\hûj\\UWUaj   jY¨U\\jûj_YWWYaj  jY¨W_jûj_W"
    "WUYYaj j[¨U_jûjaYYYYYYajj[¨S_jûjaYWUYYUQajY¨S_jûjaYU[jaQQUUQ¨S_jûjaWS\\jjaQ"
    "WUQ¨S_jûjbWS\\jûjaSQQ¨Qbjûjb_\\ajû jja\\Y\\jûûjjjjjjû  ûjjjjûû  ûûûûûû    û"
    "ûûû  ",

    // 14 = 'O'
    "    jjjjjjjjj       jj_SPPPPPU_jj    jbPWW\\\\bbaY[\\bj   jQSWbjjjjjd[\\Wjû"
    " jaUW\\jûûûûûj_\\Yaj j\\UW\\jû    j_a_ajûj\\UU\\jû    j_bbajûj\\USYjû    ja"
    "bbajû jQQSWjjjjjY_aajûû jb[QWPPPSU\\a\\djû   jj__\\[_dabbjjûû    ûjjjjjjjjj"
    "ûûû       ûûûûûûûûû    ",

    // 15 = 'P'
    "jjjjjjjjjjjjj   j\\PQPPPPPQQQaj  ja\\_\\___\\[[UQaj jjjjjjjjjj\\WSYjûjjjjjj"
    "jjjj\\UW\\jûj[QPPPPPSQSSWSjûj\\UYY\\aa_[SSSjûûj[UWWjjjjjjjjûû jYWUYjûûûûûûû"
    "û  jYWUYjû         j_aa_jû         jjjjjjû          ûûûûûû         ",

    // 16 = 'Q'
    "    jjjjjjjjj       jj_SPPPPPU_jj    jbPWW\\\\bbaY[\\bj   jQSWbjjjjjd[\\Wjû"
    " jaUW\\jûûûûûj_\\Yaj j\\UW\\jû    j_a_ajûj\\UU\\jû   jj_bbajûj\\USYjû  jbba"
    "bbajû jQQSWjjjj\\Y_aajûû jb[QWPPPSU\\a\\daj   jj__\\[_dabbab\\jû   ûjjjjjjj"
    "jjjjjjû     ûûûûûûûûûûûûû",

    // 17 = 'R'
    "jjjjjjjjjjjjj   j\\PQPPPPPQQQaj  ja\\_\\___\\[[UQaj jjjjjjjjjj\\WSYjûjjjjjj"
    "jjjj\\UW\\jûj[QPPPPPSQSSWjjûj\\UYY\\aa_[SSUjjûj[UWWjjjjaQSU[jûjYWUYjûûûjQSS"
    "\\jûjYWUYjû  jSQQYjûj_aa_jû  j\\a__jûjjjjjjû  jjjjjjû ûûûûûû   ûûûûûû",

    // 18 = 'S'
    "  jjjjjjjjjjjjj  jYSQQQQUYU[_ajûjbUU\\bdbaaafhdjûj_UU\\jjjjjjjjjjûj_US\\jjj"
    "jjjjjûûûjbUU[SQQQUUYaj   jadada_add\\\\bj   jjjjjjjjd\\_ajûjjjjjjjjjjb\\_aj"
    "ûj\\QPPSQSYWY\\\\bjûjaa_adaba__adjûûjjjjjjjjjjjjjûû  ûûûûûûûûûûûûû  ",

    // 19 = 'T'
    "jjjjjjjjjjjj j\\QPPPPQQQWjûjbbYYPSY_bbjûjjjjWQS[jjjjû ûûjWSYYjûûûû   jWPW\\"
    "jû      jWPW\\jû      jWQU\\jû      jWSS[jû      jYSSYjû      j_\\\\_jû    "
    "  jjjjjjû       ûûûûûû   ",

    // 20 = 'U'
    "jjjjjj    jjjjjj j[QQ_jû   j\\UQYjûjabbbjû   ja_WYjûjbbbajû   j_[[[jûjbbbaj"
    "û   j_\\\\\\jûjdbaajû   jbaa_jûjdba_jû   jbba_jûjbba\\jû   jba_bjû jbaYUjjj"
    "ja_\\\\jûû jf\\\\QPQUU\\[Ydjû   jjdaabfd_djjûû    ûjjjjjjjjûûû       ûûûûûû"
    "ûû    ",

    // 21 = 'V'
    "jjjjjj    jjjjjj j[PPYjû   jYQPPjûj_UW[jû   jYUU\\jû jYWW_j  j\\SWWjûû j\\W"
    "U\\jû jYUW\\jû   jYYY_jj\\WUSjûû   j\\UYY__SWQ\\jû     jUWWQQUUQjûû     j\\"
    "UYUQWU\\jû       jUY[USQjûû       j_bbd\\[jû         jjjjjjûû          ûûûû"
    "ûû     ",

    // 22 = 'W'
    "jjjjj      jjjjj jYQWjû     j_Y\\jûj[W[jû jj  jdfajûj[W[jûjQQj jbd_jûj[W[jj"
    "Q[\\Qjjab_jûj\\[[jQ\\[[[Uja__jûj[UYSU[[Y\\bb_\\_jûjYSSQS\\db_bb\\_\\jûjYWUQ"
    "Sdjjbaa\\_\\jûjaWSQ\\jûûjaY\\\\_jû jbadjûû  jfdfjûû  jjjûû    jjjûû    ûûû "
    "     ûûû  ",

    // 23 = 'X'
    "jjjjj      jjjjj jUUPPj    jQQPQjûjb_WQSj  jQWUW_jûjja[USSjjSWYW_jûû ûjaYQW"
    "PPWUY_jûû    jaQ[YWYY_jûû     jQW[QYYQ_jû     jaYQWPPWUY_j   jja[USSjjSWYW_"
    "j  jb_WQSjûûjQWUW_j jUUPPjûû  jQQPQjûjjjjjûû    jjjjjû ûûûûû      ûûûûû",

    // 24 = 'Y'
    "jjjjj      jjjjj jUUPPj    jQQPQjûjb_WQSj  jQWUW_jûjja[USSjjSWYW_jûû ûjaYQW"
    "PPWUY_jûû    jaQ[YYYY_jûû      j\\[YYW\\jûû        j_YY[jûû         jaYWYjû"
    "          jb\\WYjû          jba__jû          jjjjjjû           ûûûûûû     ",

    // 25 = 'Z'
    "jjjjjjjjjjjjjjjj jWPPPQPPPPQPQQ[jûj_\\[\\_\\YPWWUUY\\jûjjjjjjjjSWWUUUYjû ûû"
    "ûûjjUQWWUSajjû   jjUPUWY[ajjûûû jjUQSQUSajjûûû  j[PUUUQajjûûû    jYSSWSjjjj"
    "jjjjjj jYQQSWQPPPQPPPWjûj__[\\\\\\[Y\\_\\[_djûjjjjjjjjjjjjjjjjû ûûûûûûûûûûû"
    "ûûûûû",

    // 26 = '.'
    "                                                jjjjj jYUQjûjba_jûjjjjjû ûû"
    "ûûû",

    // 27 = '!'
    "jjjjjj jcPQcjûjZSSZjûjZUQZjûjZUSZjûjZUUZjûjc\\_cjûjjjjjjûjjjjjjûja[Wajûjba_"
    "bjûjjjjjjû ûûûûûû",

    // 28 = '’'
    " jjjj  jQQjû jW[jûj__jûûjjjûû  ûûû                                         "
    "   ",

    // 29 = '-'
    "                                    jjjjjjjj h\\QPQQWjûjbbY_bbjûjjjjjjjjû û"
    "ûûûûûûû                                    ",

    // 30 = '/'
    "     jjjj     jaQWjû    j[Q[jû   jaQWjûû   j[Qajû   jaQWjûû   jWQajû   jaQW"
    "jûû   jWQajû   jaQWjûû   jWQajû    jjjjûû     ûûûû     ",

    // 31 = '0'
    "  iiiiiii    iRP¨¨¨VWi  i[TV__a[W_i i[V[iii[W[iûi[V[iûi[V]iûi[T[iûi_W]iûi[T"
    "]iûi]V]iûi[T]iûic[]iûi[VYiii___iûi_VRRTW__eiû iec___ceiûû  iiiiiiiûû    ûûû"
    "ûûûû  ",

    // 32 = '1'
    "   iiii   i¨PPiû i¨[UUiûiR[XUUiûiiiXUUiû ûiXUWiû  iRRUiû  iRRUiû  iURWiû  i"
    "URRiû  i_\\aiû  iiiiiû   ûûûûû",

    // 33 = '2'
    "iiiiiiiii   iWR¨¨¨QWWi  i_a___\\WXXi iiiiiiiXXWiû iiiiii[[Uiû iRUPPRXXWiûiR"
    "XX\\_\\_aiûûiU\\\\iiiiiûû iW\\Xiiiiiii iQXXQ¨¨¨¨Qiûi\\_ba[\\_aaiûiiiiiiiiii"
    "iû ûûûûûûûûûûû",

    // 34 = '3'
    "iiiiiiiii   iWQQ¨¨PRUi  iX\\_\\\\\\\\\\[i iiiiiiic_\\iû ûûûiiifa_iû    i¨R\\"
    "a_iû    i\\baa_iû    iiifcaiûiiiiiiiccaiûiUWUWWWbbaiûibbabbfffiûûiiiiiiiii"
    "ûû  ûûûûûûûûû  ",

    // 35 = '4'
    "iiiii iiiii iRRXiûiQRXiûiRX_iûiQW\\iûiW\\XiûiQW_iûiXWUiiiQ[aiûiUUQ¨¨¨¨\\aiû"
    "i\\_[\\_\\¨\\aiûiiiiiii¨Xaiû ûûûûûi¨X_iû      iQW_iû      i[_fiû      iiiii"
    "û       ûûûûû",

    // 36 = '5'
    "jjjjjjjjjjj jVR¨¨¨¨RTWjûjWVVVY]Y]]jûjYT_jjjjjjjûjYV_jjjjjûûûjYVTR¨RVWj  j]]"
    "____VWYj jjjjjjjY]ajûjjjjjjj[aajûjV¨¨¨¨V[aajûj]]]]__aejûûjjjjjjjjjûû  ûûûûû"
    "ûûûû  ",

    // 37 = '6'
    "  jjjjjjjjj  j]T¨¨RRTWjûj[Wa_][]_ajûjYWajjjjjjjûjVVajjjjjûûûjTWVTTVW]j  jTW"
    "W]__WW]j jTYWjjjYTVjûjW][jjj[VWjûj[WT¨¨¨TV]jû j[]]_][]jûû  jjjjjjjûû    ûûû"
    "ûûûû  ",

    // 38 = '7'
    "jjjjjjjjj   jYVR¨¨¨RWj  jca_a_a[W[j jjjjjjj[W[jû ûûûûûj[V]jû      j_W_jû   "
    "   j]Vajû      jc[ajû      jccejû      jceejû      jaccjû      jjjjjû      "
    " ûûûûû",

    // 39 = '8'
    "  jjjjjjj    jYWVRRVVj  j]][cee[W[j j_[ejjjeV]jûja[_jjj_Vcjû jYYRTTVVjûû jY"
    "__cc]Tjû j]]cjjj_T_j jWT_jjjYTWjûj_WTP¨PVW_jû j[_a___]jûû  jjjjjjjûû    ûûû"
    "ûûûû  ",

    // 40 = '9'
    "  jjjjjjj    jaWTRRTWj  jggegggacYj jeecjjjacWjûja__jjj]cWjûj][_T¨T]cYjû ja"
    "cc_]Yc[jû jjjjjj[_]jûjjjjjjjYc_jûjWVTTT¨Wggjûjaaacc__gjûûjjjjjjjjjûû  ûûûûû"
    "ûûûû  ",

    // 41 = '‘'
    "jjjj  jQQjû jW[jû  j__jû  jjjû   ûûû                                       "
    "   "
};

char *redcharset[] =
{
    // 0 = '!'
    " //////  /±±²±/û /³³³±/û /´³²²/û /´³²²/û /´³²²/û /´´²±/û /³´³µ/û /·´´¶/û //"
    "¸¹//û  ////ûû  ////û   /··/û   /ºº/û   ////û    ûûûû                 ",
    // 1 = '’'
    "  ////   /²²/û  /µ·/û /¹¹/ûû ///ûû   ûûû                                   "
    "                                                   ",
    // 2 = ','
    "                                                                           "
    "           ////   /²²/û  /µ·/û /¹¹/ûû ///ûû   ûûû  ",
    // 3 = '-'
    "                                                                   ////////"
    "   /º´´²²·/   /¼¹¹¶¶»/û  /»ºººº»/û  ////////û   ûûûûûûûû                   "
    "                                                ",
    // 4 = '.'
    "                                                                   ////  /¸"
    "¶/û /¶µ/û ////û  ûûûû            ",
    // 5 = ':'
    "                         ////  /¸¶/û /¶µ/û ////û  ûûûû       ////  /¸¶/û /¶"
    "µ/û ////û  ûûûû                  ",
    // 6 = '?'
    " ///////////    /·²²²±±±´¶º/   /º¼¹¹¶¸¸¶¶µº/  ////////´³´¶/û ////////³³µ¶/û"
    " /·±±±²±³µ³³º/û /¹¶±²¹¶¹¹·º/ûû /º¶³±//////ûû  /º¶´¶/ûûûûûû   //¸¹//û       "
    "  ////ûû         ////û          /··/û          /ºº/û          ////û        "
    "   ûûûû                                      ",
    // 7 = 'A'
    "         /////            /½µ³³³µ½/         /µ¶··¶¶µ¶»/       /¶¹¸µ¹¹¹·µ¶½û"
    "     /»¸¸¸¹///¹µ·»/     /¹¸··¼/ /¹´·¹/û   /»¹¶¶¸/  /º´·¹/û   /¼¹¶¹/////ºµ·¹"
    "/û  /º»»·¹//²²³µµ·¹/û  /º»»¹/û/¹¹¹¸µ·¹/û /»ººº¹/û////¸µ¸¹/û /»º»º/ûû ûû/¹¶¸"
    "¹/û/¼»¹º¹/û    /¹µ¸¹/û/»»»»/ûû    /¼»»»/û//////û     //////û Èûûûûû      ûû"
    "ûûûû                                      ",
    // 8 = 'B'
    "////////////     /·´³³´´´´µµ·//   /¸·¸ººº¹¹¸·¸·¹/  /»º»¼¼¼¼»»º¸·¹¼/ ///////"
    "////¹¸¹¸/û///////////¹¸¹¸/û/·µ´µ´´´´µµ·¹¹¼/û/¸µµ¹ºº¹¹¸·¸·¹//û/¸µµ¹¼¼¼»»º¸·¹"
    "¼/û/¸µµ¹//////¹¸¹¸/û/¸µµº//////¹¸¹¸/û/¸¶¶µ¶´´··¸»¹¹¼/û/¸¶µ¶¸¹¹ºº»»¼»/ûû/¹¸¹"
    "»º»¼¼¼¼¼//ûû ////////////ûûû   ûûûûûûûûûûûû                                "
    "      ",
    // 9 = 'C'
    "    /////////       //º¹·¶¶µµ·¹//    /½º»¼»º¹ºº¹½ºº/   /½-¼»º¼¼¼¼¹¼¼»¹/ /½½"
    "»½¼¼////º¼½¼º/û/»»½½¼/ûûûû//////û/½¼¼¼/ûû    ûûûûûû/½.½¼/û           /½,¼¼/"
    "û           /»º¼»½/    ////// /½º»º¼¸////¸¸¹¹¹/û /º»º¼½¸µ¶·¹·ºº¼/û /½¹»¼¼ºº"
    "¹·¸·¹¼/ûû  //¼»¼¼¼¼¼»¼//ûû    û/////////ûûû       ûûûûûûûûû                "
    "                        ",
    // 10 = 'D'
    "////////////     /¹·µµµµ³³µ¸»//   /ºº¹ºº¹¹º¹¹¸º½/  /»»»»»¼¼¼º¹¹¸¸/û ///////"
    "//¼º¸¹¹»/ //////ûûû/¼ºº¹¸/û/¸²²µ/û   /¹»»¹/û/¹µµ¸/û   /¸»»¼/û/¹µ¶¸/û   /¸»º"
    "»/û/¹µµ¸/û  /»¹»º»/û/¸µµ¸////»¸º»º½/û/·µµµµ²²²·º¼»»/ûû/·µ³´´´´´·º¼¼½/û /º»»"
    "»»»»»»»»//ûû ////////////ûûû   Èûûûûûûûûûûû                                "
    "      ",
    // 11 = 'E'
    "    ////////////   //»µ´²²²³³³µ¹/û /¼¸·¶¶µ¶¶··¹ºº/û /¸µµ»»»»»»»»»»/û/»¸´µ//"
    "/////////û/¹·´µ/////ûûûûûûû/¹µ³µ´²²´/û      /¹µ³µ¶¶µµ/û      /¹µ²¶¹»»»/û   "
    "   /¹¶²µ/////û      /¼µ²µ///////////  /¸³µ´´³±±±²³´¶/û /¼¸µ¶¶¶µ¶¶¶¶¶¸/û  //"
    "»»»»»»»»»»»/û   û////////////û     ûûûûûûûûûûûû                            "
    "      ",
    // 12 = 'F'
    "    ////////////   //»¸·µµµµµµ¸º/û /»¹····¸¸¹¹¸¸¹/û /¶¹¶»»»»»»»»»»/û/»¶¹¸//"
    "/////////û/¸¶¹¸/////ûûûûûûû/¹¶¹¸¶´²´/û      /¹¶¹·¸·¸·/û      /¸¶¹·»»»»/û   "
    "   /¹¶¹¹/////û      /¹¶¹º/ûûûûû      /º¶¹¼/û          /º¶¹¼/û          /»»»"
    "»/û          //////û           ûûûûûû                                      "
    "      ",
    // 13 = 'G'
    "    ////////       //·µ´´±±²µ//    /¼¶¶µ¶¶··¹ºº¸/   /µ¶µ´¹»¼¼¼»¹¸µ/ /ºµ¶´º½"
    "////.¼¸µ/û/¸µ¶µ¼/ûûûû/////û/¸µ¶µ/ûû////////û/¸µ¶¶/û /·µ···¹/û/·¶¶¶/û /¹·¹¸¸"
    "»/û/·µ¶¶¸/ ///¹¸¹»/û/ºµ·¶µ¸////¹¹¸»/û /µ¶·¶´´´²³·¹¹»/û /¼¶¶¶µ¸¹º¼»º¹º/û  //"
    "¸¸¹ºº»¼»ºº¼/û   û////////////û     ûûûûûûûûûûûû                            "
    "      ",
    // 14 = 'H'
    "//////    ////// /¹²±¶/û   /·³³¸/û/º¶¶¹/û   /º·¸¹/û/º··º/û   /º¸¸¹/û/¹··º/û"
    "   /º¹¹º/û/¹¸¸º//////»º¹º/û/¹¶¶¹¶´²²··»»»º/û/¹µµ·¸¸¸¹¹ºº»»¼/û/¹µ¶·ººº»¼¼»»º"
    "¼/û/¹µµ¹//////¼»º¼/û/¸µµ¹/ûûûû/¼»º»/û/·µµ¸/û   /¼»º¼/û/·µ³·/û   /¼»¹»/û/º¸¸"
    "º/û   /,½»¼/û//////û   //////û ûûûûûû    ûûûûûû                            "
    "      ",
    // 15 = 'I'
    "////// /¶´³·/û/¸µ¶¹/û/·´µ¹/û/¶´µ¹/û/·´´¹/û/·µ´¹/û/¸µµ¹/û/¸µµ¹/û/¸µµ¹/û/¸µµº"
    "/û/¸¶¶º/û/¸¶µº/û/¹¸¹»/û//////û ûûûûûû              ",
    // 16 = 'J'
    "          //////           /¶²³¹/û          /¹¶¶º/û          /º··º/û       "
    "   /º··¹/û          /º¸¸¹/û          /¹¶¶¹/û          /¹µµ¹/û          /¹¶µ"
    "¹/û          /¹µµ¹/û//////////¹¶µµº/û/º¹ºº¹¸·¶¶µµµ¸/ûû/º¸·¸¹ºº¼º´·³¹/û /¼»½"
    "½¼º¼,¼º¹//ûû ////////////ûûû   ûûûûûûûûûûûû                                "
    "      ",
    // 17 = 'K'
    "//////      ////// /¸µµ¹/û    /´µµ´¸/û/¹»¼½/û   /¶º¹½¼/ûû/¹º»½/û  /´¸¹¹½/ûû"
    " /ººº½/û /¶µµ¸»/ûû  /¹º»»/û/¶¹¶·º/ûû   /¹»½¼//·»º¹»/ûû    /¹»½./»¼»»º/ûû   "
    "  /¹»½,//»»»»¶/      /¹¹»º/û/½¹»º´/     /¹¹ºº/û /»»º·¶/    /¸·¹º/û  /¼¹·¹´/"
    "   /¸¶¹¼/û   /»·¸·¶/  /¹¹»-/û    /¼¹º»¹/ /½////û     //////û ûûûûûû      ûû"
    "ûûûû                                      ",
    // 18 = 'L'
    "//////           /¹³²¶/û          /º¶¶¹/û          /º··º/û          /¹··º/û"
    "          /¹¸¸º/û          /¹¶¶¹/û          /¹µµ¹/û          /¹µ¶¹/û       "
    "   /¹µµ¹/û          /ºµµ¶¹//////////  /µµµµ···¸¹ºº¹º/û /¹³´´´´´·º¼»¹º/û  //"
    "¹¸¸¸¹º¼,½»¼/û   û////////////û     ûûûûûûûûûûûû                            "
    "      ",
    // 19 = 'M'
    "  //        //    /µ¶//    //¶µ/  /»¸·±´/  /´µ¹²¼/ /º¸µ±¶±//´¸¸¸²º/û/º¸¶±¶´"
    "±±··¸¹²º/û/¹·µ±¶µ´³·¸¸º´º/û/¹µµ±¶µµ³·¸¹»´º/û/¹µµ±/¹µµº/ººµº/û/¹µ´±//¹¹//»»µ"
    "º/û/¹¶´±/û//û/º»¸»/û/¹µ´³/û ûû/º½¸º/û/¹µ´³/û   /º½¸º/û/¹µµ³/û   /º-¸»/û/»¸¹"
    "¸/û   /».¸¼/û//////û   //////û Èûûûûû    ûûûûûû                            "
    "      ",
    // 20 = 'N'
    "  //      //////  /µ·/     /¸´´¹/û/ºµ¶´/    /º¸¸»/û/ºµ¶¶´/   /¹··»/û/ºµµ¸·´"
    "/  /¹³·»/û/¹·¶¸¸¸´/ /¹³¶º/û/º¹¸¸¹··´//¹´¶¹/û/º¶¶·¸·¸·´/¹µ¸º/û/¹··¹/º¸¹·¸·µ·"
    "º/û/º¸¹º//»¹¶¶¸··¹/û/º¹º»/û/¼·¶¸··¹/û/»ºº¼/û /º¸¶µ·¹/û/»¹»¼/û  /º¸¶´»/û/¼¼¼"
    "½/û   /¼¹¹/ûû/-////û    ///ûû  ûûûûûû     ûûû                              "
    "      ",
    // 21 = 'O'
    "    //////////       //·µ´´±±²µ··//    /¼¶¶µ¶¶··¹ºº¸¶¼/   /µ¶µ´¹»¼¼¼»¹¸µ¶/û"
    " /ºµ¶´º//////¼¸µ·º/ /¸µ¶µ/ûûûûûû/·¶µ¹/û/¸µ¶µ/û     /µ¶¶º/û/¸µ¶¶/û     /·¶¶¸"
    "/û/·¶¶¶/û     /¸¸µ¹/û/·µ¶¶/û     /¹¸·¹/û/ºµ·¶µ//////¹¸¸·º/û /µ¶·¶´´´²³·¹¹¸¸"
    "/ûû /¼¶¶¶µ¸¹º¼»º¹¹¼/û   //¸¸¹ºº»¼»ºº//ûû    û//////////ûûû       ûûûûûûûûûû"
    "                                          ",
    // 22 = 'P'
    "////////////     /·´³³´´´´µµ·//   /¸·¸ººº¹¹¸·¸·¹/  /»º»¼¼¼¼»»º¸·¹¼/ ///////"
    "////¹¸¹¸/û///////////¹¸¹¸/û/·³´´¶´´··¸»¹¹¼/û/¸·¸¸¸¹¹ºº»»¼»/ûû/¹··ºº»¼¼¼¼¼//"
    "ûû /¸¶·¹///////ûûû  /¸¶¶¹/ûûûûûûû    /¸µµ¸/û          /·´´·/û          /º¸¸"
    "º/û          //////û           ûûûûûû                                      "
    "      ",
    // 23 = 'Q'
    "    //////////       //·µ´´±±²µ··//    /¼¶¶µ¶¶··¹ºº¸¶¼/   /µ¶µ´¹»¼¼¼»¹¸µ¶//"
    " /ºµ¶´º//////¼¸µ·º/û/¸µ¶µ/ûûûûûû/·¶µ¹/û/¸µ¶µ/û     /µ¶¶º/û/¸µ¶¶/û     /·¶¶¸"
    "/û/·¶¶¶/û     /¸¸µ¹/û/·µ¶¶/û     /¹¸·¹/û/ºµ·¶µ//////¹¸¸·º/û /µ¶·¶´´´²³·¹¹¸¸"
    "/ûû /¼¶¶¶µ¸¹º¼»º¹¹¼/û   //¸¸¹ºº»¼»ºº//ûû    û///////»»¹/ûû       ûûûûû/¼¼»/"
    "û             /////û              ûûûûû   ",
    // 24 = 'R'
    "////////////     /¶³´´±±±²´µ¶//   /¸·¸·µ¶¶··¸¹·»/  /ºººº¹¸¹¹¹º¸¸·/û ///////"
    "////º··»/ ///////////ºµ¸¸/û/·´´´´±²³´´µ´¶¸/û/º¸¸··¸·µµ¶µ²¶»/û/¹¸¸¸»»¹¹µ¶µ²»"
    "//û/¹··¹///-»¸µ²³¹/û/¹¶·¹/ûûû/¹´²³¹/û/¸µ¶¸/û  /¸´²´¸/û/¸¶·¸/û  /¸³³³·/û/º¹¹"
    "º/û  /º¸·¸¹/û//////û  ///////û ûûûûûû   ûûûûûûû                            "
    "      ",
    // 25 = 'S'
    "   ////////////    /¼·¶´´²³³³·¹/û  /º¹º¸¸¶·º¹·¹º/û /ºµ¸¸»ººº»»»¹»/û /¸µ·º//"
    "////////û /¸´¶º////////ûûû /º¶µµ´³²±²²´º/    /ºµµ¶·ºº¹·¶¸º/    /»¹¹º¼»»¹¶·¸"
    "º/    ////////º¸¸¶/û //////////º··¶/û /¶±±±±±³´¹·¶¸º/û /¸²³µµµ¸¸¸·µº/ûû /¹·"
    "·¸¹¹º¹º»º/ûû  ////////////ûû    ûûûûûûûûûûûû                               "
    "      ",
    // 26 = 'T'
    "//////////////// /·µµ²²²±±±²³´³¶/û/ººº»»¹¹¶´·¹¹¸¹/û/»»»»»¹¹¶´»»»»»/û//////¹"
    "·¶¸//////û ûûûû/¸¶¶¸/ûûûûûû     /¸µ¶·/û          /¸µ´·/û          /·´´·/û  "
    "        /·µ³·/û          /¸´²¶/û          /¸³²¶/û          /¸±²·/û         "
    " /¹¶¶¹/û          //////û           ûûûûûû                                 "
    "      ",
    // 27 = 'U'
    "//////    ////// /·´µ·/û   /¶³´·/û/··¹¹/û   /·µ·¸/û/¸·¸¹/û   /¸¶·¹/û/¸·¸¹/û"
    "   /¸¶¶¹/û/·¶¸¹/û   /·¶µ¸/û/·¶¸¹/û   /·¶¶¸/û/¸·¸¹/û   /·µ¶¸/û/¸¸¸º/û   /·µ¶"
    "¹/û/¸¸¸º/û   /¸µ¶¸/û/ºµ¸»¸////·µµµ»/û /³¸»¼·±±³´·¶µ/ûû /º·»¼»·´·µ¸¶º/û   //"
    "½,»»¸»º½//ûû    û////////ûûû       ûûûûûûûû                                "
    "      ",
    // 28 = 'V'
    "//////    ////// /´³´·/û   /µ´´¸/û/¶·¸·/û   /¹·¸º/û/º···/û   /¸¸·º/û /···¶/"
    "  /····/ûû /º¸¶µ/û /¶¶µº/û   /¸···//µµµ´/ûû   /º··¸//µ¶µº/û     /¸¹¹³³µ¶µ/û"
    "û     /º¸¸µµ¶¶º/û       /·µµµ¶·/ûû       /ºµ´µ¶º/û         /µ´´¶/ûû        "
    " /¸¸¸¹/û           ////ûû            ûûûû                                  "
    "      ",
    // 29 = 'W'
    "/////      ///// /·³¸/û     /¹´·/û/¹¶º/û     /º¹º/û/ºµ¹/û     /¸¸¹/û/ºµ¹/û "
    "    /¹¶·/û/º´¸/û //  /¹¶·/û/¹¶¸/û/µµ/ /¹·¸/û/¹¶¸//´ºº´//¹·¸/û/¹¶¸/³¸º¹¹´/¹¸"
    "¹/û/¸¶³³¶¸»¹»·´¸¸¹/û/·µ±´µ¶¹»»····¹/û/¸±°³¶¹//¼¹·¶¸º/û/º°°³¹/ûû/¼·µ¸»/û /¶¶"
    "¼/ûû  /¼¹º/ûû  ///ûû    ///ûû    ûûû      ûûû                              "
    "      ",
    // 30 = 'X'
    "/////      ///// /´³´´/    /²³´¶/û/¸·¹¹¸/  /´´µ·¹/û /º¸¹¼¸//´·µ¶º/ûû  /º¹¼¼"
    "¸¶¶·µ¹/ûû    /»¼¼ºº´·¸/ûû      /½¼º¸´º/ûû        /º¹¹º/ûû        /½¹¹¸·º/  "
    "      /»¹¹ºº··¸/      /º¹¹º»»¶·µ¹/    /º¸º¹»//»·µ¶º/  /¸·¹¹»/ûû/»·µ·¹/ /»»¼"
    "¼/ûû  /»¹¹¼/û/////ûû    /////û ûûûûû      ûûûûû                            "
    "      ",
    // 31 = 'Y'
    "/////      ////// /´³´´/    /²³´¶¸/û/¸·¹¹¸/  /´´µ·¹/ûû /º¸¹¼¸//´·µ¶º/ûû   /"
    "º¹¼¼¸¶¶·µ¹/ûû     /»¼¼»º´·¸/ûû       /½¼¹¸´º/ûû         /¼º··/ûû          /"
    "¼¹¶·/û           /,¸µ¸/û           /+¸´·/û           /*¸´¸/û           /»·´"
    "¹/û           /»»¸»/û           //////û            ûûûûûû                  "
    "                        ",
    // 32 = 'Z'
    "/////////////// /¹·³³³²´´¶³²³´/û/º¹·¹º·¶¶¶¶´µ·/û/»¹»»»º»ºº·µ¶º/û////////º¶·"
    "µ¹/ûû ûûûûû/·¸´·¸/ûû      /·¸¸¸º/ûû      /¶¸¹¹º/ûû      /¶¸¸¹¸/ûû      /·¸¹"
    "¹º/ûû      /·¸¹¹º»/////// /¸¸ºº¹»»¹¸µ±±¶/û/ºº¹¹»»¹¸µµ³²¸/û/»»¼¼»¼¼»»¹¸¹¼/û/"
    "//////////////û ûûûûûûûûûûûûûûû                                ",
    // 33 = 'a'
    "                                                       //////        //·¶²²"
    "³º/      /»¸¹¸º»µ·¼/    /º¸¶º//¹¶·º/û   /¸¶·///¹¶¸¹/û  /ºµµº/´´·¶·º/û  /¸µ¶"
    "//¹»¹¶¸º/û /ºµ´º////¹··¹/û /·²³/ûûû/º·¶º/û/¸µ²º/û  /º·µº/û/¸¸·/ûû  /»º¹»/û/"
    "////û   //////û Èûûûû    ûûûûûû                                ",
    // 34 = 'b'
    "                                             ////////////   /·²²²±±±±´¶º/  "
    "/º¼¹¹¶¸¶¸¶¶µº/ /////////´³´¶/û/////////³³µ¶/û/·²±±²±±³µ³³º/û/¹·±²¹¶¶¹¹³·//û"
    "/ºº³±////µ³·¶/û/¹º´±////³³µ¶/û/º¸µ´±±±³µ³³º/û/»¼»¹¶¶¶¹¹·º/ûû////////////ûû "
    " ûûûûûûûûûûûû                                ",
    // 35 = 'c'
    "                                                 //////////   //»¸¸¶²²²²¶/û"
    " /»¶¼¼¼¼¸º¸¹º/û /¸¹¼½////////û/»¸¹¼/ûûûûûûûûû/¹¸º¼/û        /¹¸º»/û        "
    "/»¸»¼/û         /¸»¼¸////////  /¼»¼»´²´²³²¶/û  //,»»¸»ººº¹/û   û//////////û"
    "     ûûûûûûûûûû                              ",
    // 36 = 'd'
    "                                                ///////////     -¸²²²²±±²´º"
    "//   /»º¹¹¹¸¸¹¶¶µ»/  /////////¹µ´¶/û //////ûûû/µ²¶»/ /¸µµ¸/û  /µ²´¹/û/¹·¸¹/"
    "û  /µ²³¹/û/¸··¹/û  /´²³»/û/¸¶¶¸////´´²·/ûû/¸·µ´²±±´µ³·»/û /º¹¹¸¸¸¸¹¹º//ûû /"
    "//////////ûûû   Èûûûûûûûûûû                                    ",
    // 37 = 'e'
    "                                                 //////////   //¹´¶³³´´³¸/û"
    " /¼·¶¶º»º¼¼¹»/û /¹µµ¹////////û/º¶µµ¹////////û/¹¶µµµµ´´´²²¹/û/¹¶¶µ¶º¼»¼º¹»/û"
    "/ºµµ´¹////////û /·²³¹////////û /¼¹³µ³²²²³´·/û  //º¸¹¹º¹ººº/û   û//////////û"
    "     ûûûûûûûûûû                              ",
    // 38 = 'f'
    "                                           ////////   //º´±±±²·/û /¼º¶¹¹¹¸¸"
    "º/û /¸ºµº//////û/»¸¸µ////ûûûû/¹·µ´±±¶/û   /¹¶´´¹¸º/û   /¸·µ¹////û   /¹¶³¸/û"
    "ûûû   /¹¶²¸/û      /ºº·º/û      //////û       ûûûûûû                       "
    "         ",
    // 39 = 'g'
    "                                                       ////////////   //»¸¸"
    "µ´²²²²´¹/û /¼¸¹¼½¼¼¸º¸¹¹»/û /¶¸¹¼//////////û/»·¸¹/ûûûûûûûûûûû/º¸¸º/û   ////"
    "// /º¸¸º/û   /¶±±¸/û/ºµ¸»/û   /¸²±¹/û /³¸»¼/////¹²±¹/û /¼·»¼·²±²³¸²±¹/û  //"
    "½,»»¸»ºº··º/û   û////////////û     ûûûûûûûûûûûû                            "
    "      ",
    // 40 = 'h'
    "                                                //////   ////// /¸´´·/û  /·"
    "²²·/û/º¹¸¸/û  /¸¶µº/û/º¸¸¹/û  /¸µ´¹/û/º¸¸¹/////¸µ²¹/û/º¸¸¸¶´²²³¶µ²·/û/º··ºº"
    "º¹¹¹¹µ²·/û/¹¶·º/////¹´²·/û/¹µ¶º/ûûû/¹´²·/û/¹¶·¹/û  /¸³³·/û/º¹¹»/û  /º¸·º/û/"
    "-//-/û  //////û Èûûûûû   ûûûûûû                                ",
    // 41 = 'i'
    "                     ////// /·³³¸/û/¹¹¸»/û/¸º¸º/û/¹ººº/û/¹º¹º/û/¹»¹»/û/¸¹»»"
    "/û/¸·¹º/û/¸µ¸¹/û/º¹ºº/û//////û Èûûûûû              ",
    // 42 = 'j'
    "                                         //////      /·²´º/û     /º¶¹¼/û   "
    "  /»¸º»/û     /»º»º/û     /º¹º»/û     /¹¹¸º/û     /»¸¶¹/û/////¹»¹¸º/û/¶³´¶¶"
    "¹¹º/ûû/¹»¹¹»»»//û ////////ûûû  ûûûûûûûû                           ",
    // 43 = 'k'
    "                                                   //////   //////  /¶²±µ/û"
    " /±±±±¸/û /·¶³´/û/±´¶µº/ûû /¶³´¶//²´´´¸/ûû  /·²±·/²µ´³¹/ûû   /¹³²´µ¶·´´/ûû "
    "   /¸³²·/¹´³³²/     /·´²·//¸³²³µ/    /¸µ²¶/û/¸²³¸´/   /¶µ³¶/û /·´¸·µ/  /¹¸¸"
    "¹/û  /»¼º»¶/ //////û   //////û ûûûûûû    ûûûûûû                            "
    "      ",
    // 44 = 'l'
    "                                    //////      /º´²·/û     /¼¹¶»/û     /»º"
    "º»/û     /»»º»/û     /»¸¹º/û     /º¸¸¹/û     /¹¶¹»/û     /¹¶¸»¹/////  /¸¹¹²"
    "³´³¶/û //»»·¹¹¸¹/û  û////////û    ûûûûûûûû                        ",
    // 45 = 'm'
    "                                                     ///      ///    /µ±¶/ "
    "   /´±³/  /¸²±µ¶/  /µ¸´¶¸/ /·³³¶»·//³¹·¶¸»/û/¸³µ¸»»´´µ·µ·¸»/û/¶³µ¶¼»º¹µ¶´¸º"
    "¼/û/¶³´·¼»º¸¶µ³ºº*/û/¶´³¸/¼º¶¹/·º»»/û/µ´´·//º¹//¸ºº»/û/µ³³·/û//û/¸»¹¹/û/¸¸¸"
    "¹/û ûû/º¼»º/û//////û   //////û Èûûûûû    ûûûûûû                            "
    "      ",
    // 46 = 'n'
    "                                                     ///     //////  /±±º/ "
    "   /¸°±¸/û/¸´µ´º/   /¶°´¸/û/¹¶µµ¶º/  /¶°µ¹/û/¹µµ´¶¶º/ /·°´¹/û/º¶¶¶¶¶¶º//·°³"
    "¹/û/º¶µ´¶¶´²º/¶°³¹/û/º¶´·/º²²´´²°³¹/û/ºµ³¸//º²µ´²°³¹/û/»µ³¸/û/º³²²°²»/û/»¹¸"
    "º/û //º¸¶¸/ûû//////û  û////ûû  Èûûûûû    ûûûû                              "
    "      ",
    // 47 = 'o'
    "                                                          /////////       /"
    "/¹³±±±±±´¹//    /»±µµ¸¸»»º¶·¸»/   /²³µ»/////¼·¸µ/û /º´µ¸/ûûûûû/¹¸¶º/ /¸´µ¸/"
    "û    /¹º¹º/û/¸´´¸/û    /¹»»º/û/¸´³¶/û    /º»»º/û//²²³µ/////¶¹ºº/ûû /»·²µ±±±"
    "³´¸º¸¼/û   //¹¹¸·¹¼º»»//ûû    û/////////ûûû       ûûûûûûûûû                "
    "                        ",
    // 48 = 'p'
    "                                             ////////////   /·²²²±±±±´¶º/  "
    "/º¼¹¹¶¸¶¸¶¶µº/ /////////´³´¶/û/////////³³µ¶/û/·²±±²±±³µ³³º/û/¹·±²¹¶¶¹¹·º/ûû"
    "/ºº³±///////ûû /¹º´³/ûûûûûûû  /º¸µ´/û        /»¼»¹/û        //////û        "
    " Èûûûûû                                      ",
    // 49 = 'q'
    "                                                          /////////       /"
    "/¹³±±±±±´¹//    /»±µµ¸¸»»º¶·¸»/   /²³µ»/////¼·¸µ/û /º´µ¸/ûûûûû/¹¸¶º/ /¸´µ¸/"
    "û    /¹º¹¹/û/¸µµ¸/û    /¹»»º/û/¸µ¸¶/û    /º»»º/û /¸¸µ¹/////¶¹ºº/ûû /»¹¸µ¶±±"
    "³¶¸º¸¼/û   //¹¹¸·¹¼º»»//ûû    û//////¸»/ûûû       ûûûû/º»/û             ///"
    "/û              ûûûû    ",
    // 50 = 'r'
    "                                                /////////////   /¸±²±±±±±²²"
    "²º/  /º¸¹¸¹¹¹¸··´²º/ //////////¸µ³¶/û//////////¸´µ¸/û/·²±±±±±³²³³µ/ûû/¸´¶¶¸"
    "ºº¹·³³´/û /·´µµ////º²³´·/ /¶µ´¶/ûûû/²³³¸/û/¶µ´¶/û  /³²²¶/û/¹ºº¹/û  /¸º¹¹/û/"
    "/////û  //////û Èûûûûû   ûûûûûû                                ",
    // 51 = 's'
    "                                                  /////////////  /¶³²²²²´¶´"
    "·¹º/û/»´´¸»¼»ººº½-¼/û/¹´´¸//////////û/¹´³¸////////ûûû/»´´·³²²²´´¶º/   /º¼º¼"
    "º¹º¼¼¸¸»/   ////////¼¸¹º/û//////////»¸¹º/û/¸²±±³²³¶µ¶¸¸»/û/ºº¹º¼º»º¹¹º¼/ûû/"
    "////////////ûû  Èûûûûûûûûûûûû                                  ",
    // 52 = 't'
    "                                       //////////// /¸²±±±±²²²µ/û/»»¶¶±³¶¹»"
    "»/û////µ²³·////û Èû/µ³¶¶/ûûûû   /µ±µ¸/û      /µ±µ¸/û      /µ²´¸/û      /µ³³"
    "·/û      /¶³³¶/û      /¹¸¸¹/û      //////û       ûûûûûû                    "
    "         ",
    // 53 = 'u'
    "                                                   //////    ////// /·²²¹/û"
    "   /¸´²¶/û/º»»»/û   /º¹µ¶/û/»»»º/û   /¹···/û/»»»º/û   /¹¸¸¸/û/¼»ºº/û   /»ºº"
    "¹/û/¼»º¹/û   /»»º¹/û/»»º¸/û   /»º¹»/û /»º¶´////º¹¸¸/ûû /½¸¸²±²´´¸·¶¼/û   //"
    "¼ºº»½¼¹¼//ûû    û////////ûûû       ûûûûûûûû                                "
    "      ",
    // 54 = 'v'
    "                                                   //////    ////// /·±±¶/û"
    "   /¶²±±/û/¹´µ·/û   /¶´´¸/û /¶µµ¹/  /¸³µµ/ûû /¸µ´¸/û /¶´µ¸/û   /¶¶¶¹//¸µ´³/"
    "ûû   /¸´¶¶¹¹³µ²¸/û     /´µµ²²´´²/ûû     /¸´¶´²µ´¸/û       /´¶·´³²/ûû       "
    "/¹»»¼¸·/û         //////ûû          ûûûûûû                                 "
    "      ",
    // 55 = 'w'
    "                                                   /////      ///// /¶²µ/û "
    "    /¹¶¸/û/·µ·/û //  /¼½º/û/·µ·/û/²²/ /»¼¹/û/·µ·//²·¸²//º»¹/û/¸··/²¸···´/º¹"
    "¹/û/·´¶³´··¶¸»»¹¸¹/û/¶³³²³¸¼»¹»»¸¹¸/û/¶µ´²³¼//»ºº¸¹¸/û/ºµ³²¸/ûû/º¶¸¸¹/û /»º"
    "¼/ûû  /½¼½/ûû  ///ûû    ///ûû    ûûû      ûûû                              "
    "      ",
    // 56 = 'x'
    "                                                   //////    ////// /²±²²²/"
    "  /²´´´¹/û /¸¶¶µ±//¶»»ºº/ûû  /¸³´³±±·»»»/ûû    /¸µ³µ¸»»¼/ûû      /¸³µ¶¼¼/ûû"
    "       /³³´·¼»/û       /±²´³¶»»¶/      /²³±´ºº»»º´/    /²³³±¸//º¸¸¸²/  /¸¸¸"
    "·¶/ûû/ººº¸µ/ //////ûû  //////û Èûûûûû    ûûûûûû                            "
    "      ",
    // 57 = 'y'
    "                                                   /////      ///// /´´±±/ "
    "   /²²±²/û/»¹µ²³/  /²µ´µ¹/û /º·´³³//³µ¶µ¹/ûû  /º¶²µ±±µ´¶¹/ûû    /º²·¶¶¶¶¹/û"
    "û      /¸·¶¶µ¸/ûû        /¹¶¶·/ûû         /º¶µ¶/û          /»¸µ¶/û         "
    " /»º¹¹/û          //////û           ûûûûûû                                 "
    "      ",
    // 58 = 'z'
    "                                                   //////////////// /µ±±±²±"
    "±±±²±²²·/û/¹¸·¸¹¸¶±µµ´´¶¸/û////////³µµ´´´¶/û ûûûû//´²µµ´³º//û   //´±´µ¶·º//"
    "ûûû //´²³²´³º//ûûû  /·±´´´²º//ûûû    /¶³³µ³////////// /¶²²³µ²±±±²±±±µ/û/¹¹·"
    "¸¸¸·¶¸¹¸·¹¼/û////////////////û Èûûûûûûûûûûûûûûû                            "
    "      "
};

char *nmare =
    "  //      //////                                                           "
    "                                          //////  /²²/     /²¯±²/û         "
    "                                                                           "
    "                /²¯²²/û/µ±±²/    /³±³¶/û                                   "
    "                                                                 /³±´´/û/³´"
    "³²µ/   /³±¶·//////   /////////////////   ///////////////// ///      ///    "
    "    ////// /////////////     ///////////³±´¶/û/´´³³·µ/  /µ²´¶/¶´´·/û//º¹´´¯"
    "±±³³µ¹/·±µ·/û  /·³´¶/·³²±±²³³³µ//µ²¶/    /¯²³/     //·¶³³´º//·²³²²³²²³³³º/ "
    " //µµ¶´±µµ´·//µ²³´/û/¶´³µ¸¸µ/ /µ²´¶/´¹¹´//»¹ºº¹·±¸¹¶´¹º/º²¸¶/û  /´·²º/º±¶±±"
    "´¶¹ºº/¸³²¶¶/  /¶±µ¶¹/   /º¸¹¸³º¶·»/º¸´±´¹·´·²µ³º//»·´³ºº²º³¹º//µ²²´/û/³³µ¶¹"
    "··µ//¶²µ¶/´¸·´//··º»/»³º//////º³¸¶/û  /´º¯¹/ºµ/±³´·////·´´¶º³//µ¹µ¶¸º/û /º¸"
    "¶º//¹¶·º///¹²¹/¼¶/¶µ´¶//¹¶²¹¼ºµº¶¼-//·²³·/û/±²¶·¸·¸·µ/¶²¶¸/²·¶²/º··¹/û/¸/ûû"
    "ûûû/ºµ¸¹/////²µ±¹/º¸/´´¶¶/ûû/¸´¶¶º¯µµ¶·¹¶¶º/û /¸¶²///¹¶¸¹////´¹/¼¹/¹´¶¸/º¶´"
    "¯¹//¹/¹////¸´¶¹/û/¶µ´¹ºº¹¹¸¹º±¸º/±µ¶¯/º¹·¹/û »û//////º´¹·¶±³³´¯µµ¸//¹-¶²¶¹/"
    "û /¶´¶´»±¹¹¶·¹¶º»/û/º¶¶µº²µ¸´¸º/¸¯±²²¯²´³´²¶//¹µ¶±¶µµµ±³¹¹///¹¹//û/¶µ¶º/¼»º"
    "µ¶ºµ¹»/±´¶±/º¹¶º/û  û/¶ºµ¹/º²º¸º¶º´¹±³¶¸/û-/´¯¶¹/û /´´¶²»¹º¹¶¸´¶º¶/û/¹µ¶º/³"
    "¶º²·º/¹²¹µ¹º²º¹´¯´//ºµ¶¯¶º´¹²¹º»/û////ûû/¶·¹¹//¼»³ºº´¹»/µ²µµ/º¶´¹/û   /¹»¶º"
    "/º¶¶¹¼¹»´¼µ³¹¸/û /²²µ¶/û /²µ¶²/»º¶º/¹µº¹//º¶µº//¶ºº¯¶º/´´¹³//º/º³´²¹/º¶µ²º/"
    "º»´º////ºµµº/ /´¸·¸/û/¼ºµ·´¹»/·±µº/»´´¸»/////·»´º/·¶´¹///¹/º¯¶¹/û /²··´/û /"
    "µ²¶´//»º//¹²º¶//¹´µ/û/»/»´´»/·±¹´/ûûû/´´±¹//¹¶´º///¹/////µ²¹µ/û/´¸´¹/û /¼´¹"
    "±¹»/´±¶»//¹¶·¼¹µ²´»º»´º/´¹´¹//ûûûº´²¹/û /»´¹²/û /¶²´´/û//û/¹²¹´/¹¶µ»/û //»²"
    "´»/¶¶»´/û  /´´±·/û/¹´¹´´´³µ·¼///º///û/·¹´¹/û  /¼¹µº½/´¶·»/û/¹¸-»»²·»»¼·»/´º"
    "·¹/ûû  º´²»/û /º´¹º/û /¹±¹²/û ûû/»·»µ/¹µ»/ûû  /»²·»/µ´¼´/û  /¹µ²º/û /¶»¸µ¸º"
    "¸»¼//´/º´/û/¹º·¹/û   /-º¼-/´»µ/ûû/ºº»////»·-´//µ/µ»/û   º·º/ûû /»²»»/û /»·º"
    "¶/û    /´/»//µ/ûû   /»·´//ºµ/·/û   /·³/ûû /¹»/·//·»/ûû/¶¸/ûû/¹»¹&ûû   /-»½/"
    "/µ/º/û  /¼/ûûûû/µ/»//´/»/ûû   /º/ûû   -µ-/ûû  /µ»/ûû    /µ//û/»/û    /»´»/û"
    "/»/º/û    /³/û  /º/û»//»/ûû ////û /»)»/û     /-/û/»//ûû  /»/û   /µ//û/»//ûû"
    "     /ûû     /ûûû   /µ/ûû      /ûûû /ûû     /»/ûû /û»/û    /¸/û   /ûû/ûû/ûû"
    "   ûûûû  /û//û      /ûû /ûûû    /ûû    /ûûû /ûûû       û       û      /ûû  "
    "      û    û       /ûû   û/ûû     /ûû    û  û  û           û/ûû       û   û"
    "       û      û    û                         û                       û     "
    " û       û                         û                                       "
    "                                                                           "
    "     ";

int lsleft[] =
{
    159,  10,  10,  10,  10,   9,  10,   9,  10,  11, 126,  11, 126, 126, 126,
    126,  10, 126, 239, 239, 239, 239, 239, 239,  10,  11, 238, 151, 151, 151,
     77, 151,  10, 126, 238, 151,  76, 151,  78, 151,   9, 126, 238,  76, 151,
    151,  77,  77,  10, 126, 238,  77, 151,  77,  77,  77,   9, 126, 238, 151,
    151,  77,  78,  77,   9, 126, 238,  77,  78,  77, 151, 151,   9,  11, 238,
    151,  77, 151,  77,  77,  10, 126, 238, 151, 151,  78, 151,  78,   9, 126,
    238, 151, 151, 151,  77,  78,  10, 126,  10, 159,   9,   9, 159, 159,   9,
     10,  10,  10,  10,   9,  10,   9,  10,  11, 126,  11, 126, 126, 126, 126,
      0, 251, 251, 251, 251, 251, 251, 251
};

int lscntr[] =
{
      9,  10,   9,  10,  10,   9,  10,   9,  11, 126, 126,  11, 126, 126, 126,
    126, 239, 239, 239, 239, 239, 239, 239, 239, 151,  77,  78,  77, 151,  77,
     78,  77, 151,  77, 151,  77,  77,  78,  77,  78, 151,  78,  77,  78,  77,
     77,  77,  77,  77,  77,  78,  77, 151,  77, 151,  77,  77,  77,  78,  77,
     77,  77,  77,  77, 151,  78,  77,  77, 151,  77,  78,  77,  77,  77,  77,
     77,  78,  77,  78, 151,  78,  77,  78,  77,  77, 151,  77, 151,  77,  78,
     77, 151,  77,  78,  77, 151,   9, 159,   9, 159,   9,   9, 159, 159,   9,
     10,   9,  10,  10,   9,  10,   9,  11, 126, 126,  11, 126, 126, 126, 126,
    251, 251, 251, 251, 251, 251, 251, 251
};

int lsrght[] =
{
      9,  10,   9,  10,   9,  10,  10, 159,   0,  11, 126, 126, 126, 126, 126,
     11,  10, 251, 239, 239, 239, 239, 239, 239, 126,  10, 251,  78,  77, 151,
    151, 151, 238,  11,  10, 251,  78, 151,  78, 151, 151, 238, 126,  10, 251,
     77,  77, 151,  77, 151, 238, 126,   9, 251, 151, 151,  76,  76, 151, 238,
    126,  10, 251,  76,  76, 151,  77, 151, 238, 126,   9, 251, 151,  77, 151,
     76,  76, 238, 126,   9, 251,  77,  77, 151, 151,  76, 238,  11,   9, 251,
    151,  78, 151,  76, 151, 238, 126,  10, 251, 151,  77, 151, 151, 151, 238,
    126,   9, 251,   9, 159,   9, 159,   9,   9, 126,  10, 251,   9,  10,   9,
     10,   9,  10,  10,   9, 251,  11, 126, 126, 126, 126, 126,  11,  10, 251,
    251, 251, 251, 251, 251, 251, 251, 251, 251
};

char *savecaret = "////³//³//³//³//³//³//³////";

char *smallcharset[] =
{
    // 0 = '!'
    "     //// /±±/û/µ²/û/µ±/û////û/µ³/û////û ûûûû     ",
    // 1 = '”'
    "/////// /µ³/µ³/û//¶//¶/û //////û  ûûûûûû                                   "
    "     ",
    // 2 = '#'
    "         /////  //µ/´// /´±±±´/û//±/±//û/µ±±±´/û//´/´//û /////ûû  ûûûûû    "
    "     ",
    // 3 = '$'
    "  ///    //µ/// /¹µ²µ¹/û/µ/²//ûû/¹µ²µ¹/  //¹/µ/û/¹µ²µ¹/û///µ//ûû û///ûû    "
    "ûûû  ",
    // 4 = '%'
    "          ///  ///  /±/û/²³/û ////µµ·/û  û/µ´·/ûû  /¶´·////  /µ·/û/±/û ///û"
    "û///û  ûûû  ûûû          ",
    // 5 = '&'
    "           ////    /¸µ¶¹/   /³-´º/û /»µµ¶/ûû /¶¶-µ´// /»·¶´´´/û ///////û  û"
    "ûûûûûû         ",
    // 6 = '’'
    "//// /µ³/û//³/û ///û  ûûû                         ",
    // 7 = '('
    "       //// /º²±/û/¸µ//û/¸µ/ûû/¸µ// /º¸³/û ////û  ûûûû      ",
    // 8 = ')'
    "      ////  /±²º/ //µ¸/û /¶¸/û//¶¸/û/³¶º/û////ûû ûûûû       ",
    // 9 = '*'
    "           /      /¶/    /¶²¶/  /¸²²²¸/  /¸²¸/ûû  /¸/ûû    /ûû      û           ",
    // 10 = '+'
    "             ///  //¶// /¶¶¶/û//¶//û ///ûû  ûûû             ",
    // 11 = ','
    "                         //// /µ³/û//³/û ///û  ûûû",
    // 12 = '-'
    "                  ///// /±±±/û/////û ûûûûû                  ",
    // 13 = '.'
    "                         //// /µ³/û////û ûûûû     ",
    // 14 = '/'
    "            ///    /µ¶/û  /µ¶¹/û /µ¶¹/ûû/´¶¹/ûû /¸º/ûû  ///ûû    ûûû       "
    "     ",
    // 15 = '0'
    "          //////  //¸µµ¸// /¸³¹¹´¸/û/¸µ//²¶/û/¸´¹¹´¸/û//¸µµ¸//û //////ûû  û"
    "ûûûûû          ",
    // 16 = '1'
    "       //// //±±/û/¶µ³/û//µ¶/û /µµ/û /µ³/û ////û  ûûûû      ",
    // 17 = '2'
    "         ///////  /µ´µ²³¼/ ///¼¹³¹/û/¼·µ´µ¼/û/µ´////ûû/µ´µ³±µ/ ////////û ûû"
    "ûûûûûû         ",
    // 18 = '3'
    "         //////   /¸³³´¹/   ///¹³¹/  /´´´²·/û ///¹µº/û/·´²µ·/ûû//////ûû  ûû"
    "ûûûû           ",
    // 19 = '4'
    "        /////// /³´/´´/û/³µ/³³/û/³³±±³/û////²´/û ûû/³´/û   ////û    ûûûû   "
    "     ",
    // 20 = '5'
    "        /////// /¶²²²´/û/¶²///ûû/¶²²µ¹/ //¼¼´²/û/¸±³µ¹/û//////ûû ûûûûûû    "
    "     ",
    // 21 = '6'
    "           //////  /·´²²¹/û/·´»///ûû/·²´²²»/ /·²//±¸/û /··²²»/û  /////ûû   "
    "ûûûûû          ",
    // 22 = '7'
    "         ///////  /µ´µ²²·/ ////¹¶µ/û ûûû/µ´/û    /µ²/û    /µ³/û    ////û   "
    "  ûûûû         ",
    // 23 = '8'
    "          //////  /º·³´µº/ /º´//µº/û /´¶¶´/ûû/º´//´º/ /º···µº/û //////ûû  û"
    "ûûûûû          ",
    // 24 = '9'
    "          /////   /»´°³·/  /¶±//µº/  »¶¶¶´·/û////»´·/û/¹²²´·/ûû//////ûû  ûû"
    "ûûûû           ",
    // 25 = ':'
    "     //// /±¸/û////û ûûûû//// /²¸/û////û ûûûû     ",
    // 26 = ';'
    "          //// /±±/û////û////û/µ³/û//³/û ///û  ûûû",
    // 27 = '<'
    "         //   /µ/û /µ´/û/µµ/ûû /µ´/   /µ/û   //û    ûû      ",
    // 28 = '='
    "            ///// /±±±/û/////û/µ³³/û/////û ûûûûû            ",
    // 29 = '>'
    "      //    /µ/   /´µ/   /µµ/ /´µ/ûû/µ/ûû //ûû   ûû         ",
    // 30 = '?'
    "         ///////  /µ´µ²³¶/ ////¶³¹/û/µµµµµ¹/û///////ûû/µ´µ/ûûû /////û    ûû"
    "ûûû            ",
    // 31 = '@'
    "  /////    /·²²²µ/  /·»////µ/ /²/»·²/²/û/²/²/²/¶/û/²/²²²¶»/û/·»//////û /·²²"
    "²µ/ûû  /////ûû    ûûûûû  ",
    // 32 = 'A'
    "           /////   /·²²²¶/ /º´//¶³/û/´·µµ´³/û/´·//´³/û/´·//´³/û////////û ûû"
    "ûûûûûû         ",
    // 33 = 'B'
    "         ///////  /¶³±²±¹/ ////º³¹/û/·µ¶µµ/ûû/µµ/º³·/ /µ´²²´»/û///////ûû ûû"
    "ûûûûû          ",
    // 34 = 'C'
    "           //////  /º³±±´/û/¹³³////û/¸µµ/ûûûû/º³³////  /ºµ±±¶/û  //////û   "
    "ûûûûûû         ",
    // 35 = 'D'
    "         //////   -¸²±´¹/  ////»¶¹/ /µµ//´¶/û/´²/»´¹/û/´²±³¹/ûû//////ûû  ûû"
    "ûûûû           ",
    // 36 = 'E'
    "           //////  /¹µ´³¸/û/¹³³¹///û/·²³´µ¹/û/ºµ³¹///û /·³µ´·/û  //////û   "
    "ûûûûûû         ",
    // 37 = 'F'
    "         //////// /µ´±±±±/û/µµ¹////û/µµµµµ/ûû/µ¶////û /µ·/ûûûû ////û     ûû"
    "ûû             ",
    // 38 = 'G'
    "           //////  /·´²²²/û/·´¸////û/·´»/´´/û/ºµ¸/¸²/û /··²µ²/û  //////û   "
    "ûûûûûû         ",
    // 39 = 'H'
    "         //////// /´·//·²/û/´·//µ´/û/´´µµµ²/û/´·//·²/û/´·//¸³/û////////û ûû"
    "ûûûûûû         ",
    // 40 = 'I'
    "     //// /·±/û/·²/û/·³/û/·²/û/·³/û////û ûûûû     ",
    // 41 = 'J'
    "             ////     /²¸/û    /µ¸/û    /¶¸/û////¹¶¸/û/¶³³³¶º/û///////ûû ûû"
    "ûûûûû          ",
    // 42 = 'K'
    "         //// /// /·±//±¶/û/·´/²¸/ûû/·²´´/ûû /·²/¸³/  /·³//¸³/ ////û///û ûû"
    "ûû ûûû         ",
    // 43 = 'L'
    "         ////     /´²/û    /¸µ/û    /¸µ/û    /¸µ¹//// /º¸³³³³/û ///////û  û"
    "ûûûûûû         ",
    // 44 = 'M'
    "           //   //  /¹´/ /³¹/ /·´·/¹µ³/û/µ¶·´·µ·/û/¶¸/¶/µ·/û/µ·///µ·/û////û"
    "////û ûûûû ûûûû          ",
    // 45 = 'N'
    "          /////// /º´»/´³/û/µµ¸/¶³/û/µ´··³³/û/µ·»´²µ/û/µ¸/º²»/û///////ûû ûû"
    "ûûûûû          ",
    // 46 = 'O'
    "          //////  //¸µµ¸// /¸³¹¹´¸/û/¸µ//²¶/û/¸´¹¹´¸/û//¸µµ¸//û //////ûû  û"
    "ûûûûû          ",
    // 47 = 'P'
    "         ///////  /·²±±±º/ ////½µµ/û/±±±³³º/û/±±////ûû/µµ/ûûûû ////û     ûû"
    "ûû             ",
    // 48 = 'Q'
    "           ////    /¶µµ¶/  /¶³¹¹µ¶/ /¶µ//µ¶/û/º³¹¹·º/û /ºµµ·/ûû  ///··/    "
    "ûû///û      ûûû",
    // 49 = 'R'
    "         ///////  /·²±±±º/ ////ºµ¸/û/³±±±³/ûû/µ±/¸µº/ /µµ//µ¸/û////////û ûû"
    "ûûûûûû         ",
    // 50 = 'S'
    "         ////// /¹¶²´¹/û/¶¶////û/¹µ¶¸¹/û////´¶/û/¸±³¶¹/û//////ûû ûûûûûû    "
    "     ",
    // 51 = 'T'
    "         //////// /°°±±´´/û///µ²///û û/µ³/ûûû  /µ²/û    /µ³/û    ////û     "
    "ûûûû           ",
    // 52 = 'U'
    "         //////// /·²//´¶/û/¶µ//µ·/û/·µ//µ·/û/ºµ³³µº/û /º··º/ûû  ////ûû    "
    "ûûûû           ",
    // 53 = 'V'
    "        /////// /´±/²µ/û/´²/³µ/û /´¹²/ûû /¶²¶/û   /´/ûû   ///û     ûûû     "
    "     ",
    // 54 = 'W'
    "          //// //// /³³///³º/û/¶´/³/´¸/û/¶³º´º´¸/û/¸³¶´¶µ¸/û/¸²¶/µ¸¸/û ///û"
    "///ûû  ûûû ûûû           ",
    // 55 = 'X'
    "          //// //// /³µ·/·µ·/û /¶·µ·¶/ûû  /·µµ/ûû  /¶·µ·¶/  /³µ·/·µ·/ ////û"
    "////û ûûûû ûûûû          ",
    // 56 = 'Y'
    "         //////// /±±//²²/û/´³//³¶/û /µ±±µ/ûû  /·¶/ûû   /·µ/û    ////û     "
    "ûûûû           ",
    // 57 = 'Z'
    "        /////// /µ´²²¶/û///´¶º/û /´µº/ûû/¶´º/// /µ´³±µ/û///////û ûûûûûûû   "
    "     ",
    // 58 = '['
    "       //// /º²±/û/¸µ//û/¸µ/ûû/¸µ// /º¸³/û ////û  ûûûû      ",
    // 59 = '\'
    "        ///     /¶³/    /¹³µ/    /¹µµ/    /¹µ´/    /¹´/û    ///û     ûûû   "
    "     ",
    // 60 = ']'
    "      ////  /±²º/ //µ¸/û /¶¸/û//¶¸/û/³¶º/û////ûû ûûûû       ",
    // 61 = '^'
    "           /      /¸/    /¸µ¸/  /¸´/´¸/ ///û///û ûûû ûûû                   "
    "     ",
    // 62 = '_'
    "                                             //////// /±±±±±±/û////////û ûû"
    "ûûûûûû         ",
    // 63 = '|'
    "     //// /±±/û/µ³/û/µ¶/û/µµ/û/µ³/û////û ûûûû     ",
    // 64 = '“'
    "/////// /³µ/³µ/û/¶//¶//û//////ûû ûûûûûû                                    "
    "     ",
    // 65 = '‘'
    "//// /³µ/û/³//û///ûû ûûû                          ",
    // 66 = '°'
    "       ///  /µ³µ/ /³/³/û/µ³µ/û ///ûû  ûûû                   ",

};

char *underscores1 =
    "               ///////                                       ///////       "
    "                ////                                         ////////      "
    "                 ////////                                                  "
    "             ////////                                                      "
    "                                   /³³³³³/û                                "
    "      /³³³³³/û                      /³³/û                                  "
    "      /³³³³³³/û                      /³³³³³³/û                             "
    "                                 /³³³³³³/û                                 "
    "                                                       ///////û            "
    "                          ///////û                      ////û              "
    "                          ////////û                      ////////û         "
    "                                                     ////////û             "
    "                                                                           "
    " ûûûûûûû                                       ûûûûûûû                     "
    "  ûûûû                                         ûûûûûûûû                    "
    "   ûûûûûûûû                                                               û"
    "ûûûûûûû                                                                    "
    "     ";

char *underscores2 =
    "            ///////                                       ///////          "
    "             ////                                         ////////         "
    "              ////////                                                     "
    "          ////////                                                         "
    "                                /³³³³³/û                                   "
    "   /³³³³³/û                      /³³/û                                     "
    "   /³³³³³³/û                      /³³³³³³/û                                "
    "                              /³³³³³³/û                                    "
    "                                                    ///////û               "
    "                       ///////û                      ////û                 "
    "                       ////////û                      ////////û            "
    "                                                  ////////û                "
    "                                                                         ûû"
    "ûûûûû                                       ûûûûûûû                       û"
    "ûûû                                         ûûûûûûûû                       "
    "ûûûûûûûû                                                               ûûûû"
    "ûûûû                                                                       "
    "     ";