/*
 * Copyright (C) 2011 Ahmad Amarullah ( http://amarullz.com/ )
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Descriptions:
 * -------------
 * AROMA File Manager Terminal
 *
 */


/* Terminal Structure */
typedef struct {
  int       fd;
  pid_t     pid;
  pthread_t rthread;
  byte      active;
  const char   *  basepath;
  ACONTROLP console;
  byte      drawnow;
} AUITERM, *AUITERMP;

/* Reader Thread */
static void * auiterm_reader(void * cookie) {
  /* Variable is in cookie */
  AUITERMP p  = (AUITERMP) cookie;
  p->active   = 1;
  
  do {
    char b = 0;
    ssize_t sz = read(p->fd, &b, 1);
    
    if (sz > 0) {
      if (b) {
        aconsole_add(p->console, b);
        
        if ((b == '\n') || (p->drawnow)) {
          aconsole_ondraw(p->console);
          p->drawnow = 0;
        }
      }
    }
    else {
      break;
    }
  }
  while (p->active);
  
  if (p->active == 1) {
    p->active = 0;
    atouch_send_message(0xffffffff);
  }
  
  p->active   = 0;
  printf("\n::: END OF READER THREAD :::\n");
  return NULL;
}

/* Release Pipes */
void auiterm_release(AUITERMP p) {
  /* Close pipes */
  kill(p->pid, SIGHUP);
  close(p->fd);
}
char auiterm_ctrl(char c) {
  if (c >= 'A') {
    return (c - ((c > 96) ? 96 : 64));
  }
  
  return c;
}

/* Send Into SHELL */
void auiterm_send(AUITERMP p, char c, byte iscursor) {
  //
  p->drawnow = 1;
  
  if (iscursor) {
    switch (c) {
      case 37:
      case 38:
      case 39:
      case 40: {
          char crd[4] = { 'D', 'A', 'C', 'B' };
          char bks[4];
          snprintf(bks, 4, "\033[%c", crd[c - 37]);
          write(p->fd, bks, strlen(bks));
        }
        break;
    }
  }
  else {
    switch (c) {
      case 8: {
          char * bks = "\177";
          write(p->fd, bks, strlen(bks));
        }
        break;
        
      case '\n': {
          if (aconsole_isclrf(p->console)) {
            char * bks = "\r\n";
            write(p->fd, bks, strlen(bks));
          }
          else {
            c = '\r';
            write(p->fd, &c, 1);
          }
        }
        break;
        
      default: {
          write(p->fd, &c, 1);
        }
        break;
    }
  }
}

/* Init Shell and Pipes */
byte auiterm_init(AUITERMP p) {
  /* Init Pipe */
  int m, s;
  struct winsize w = { 80, 24, 0, 0};
  
  if (openpty(&m, &s, NULL, NULL, &w) < 0) {
    return 0;
  }
  
  /* FORK & RUN */
  signal(SIGCHLD, SIG_IGN);
  p->pid = fork();
  
  if (p->pid == 0) {
    setsid();
    dup2(s, STDIN_FILENO);
    dup2(s, STDOUT_FILENO);
    dup2(s, STDERR_FILENO);
    ioctl(s, TIOCSCTTY, NULL);
    close(s);
    close(m);
    /* Change Current Path */
    chdir(p->basepath);
    /* Try All Possible Shell */
    execlp("/bin/bash", "bash", "-i", NULL);
    execlp("/sbin/bash", "bash", "-i", NULL);
    execlp("/system/bin/bash", "bash", "-i", NULL);
    execlp("/system/xbin/bash", "bash", "-i", NULL);
    execlp("/system/sbin/bash", "bash", "-i", NULL);
    execlp("/bin/sh", "sh", "-i", NULL);
    execlp("/sbin/sh", "sh", "-i", NULL);
    execlp("/system/bin/sh", "sh", "-i", NULL);
    execlp("/system/xbin/sh", "sh", "-i", NULL);
    execlp("/system/sbin/sh", "sh", "-i", NULL);
    _exit(-1);
  }
  
  /* Close unused pipe */
  close(s);
  p->fd = m;
  //signal(SIGCHLD, auiterm_sigchld);
  /* Start Reader Thread */
  pthread_create(&p->rthread, NULL, auiterm_reader, (void *) p);
  pthread_detach(p->rthread);
  return 1;
}

/* Drawing */
void auiterm_inittitle() {
  /* Redraw BG */
  aui_setbg(NULL);
  char termtitle[200];
  snprintf(termtitle, 200, "<@center>%s</@>", alang_get("tools.terminal.title"));
  /* Set Title */
  int pad  = agdp() * 2;
  ag_textf(&aui_win_bg, agw(), 1, pad + 1, termtitle, acfg()->titlebg_g, 1);
  ag_text(&aui_win_bg, agw(), 0, pad, termtitle, acfg()->titlefg, 1);
}

/* Show Interface */
void aui_show_terminal(const char * basepath) {
  auiterm_inittitle();
  /* Calculating */
  int pad  = agdp() * 2;
  int btnH = agdp() * 20;
  int wrkH = agh() - (aui_minY + btnH + pad * 2);
  int conH = round(wrkH / 1.75);
  int imeH = wrkH - conH;
  int btnW = (agw() - pad * 2) / 4;
  int btnY = aui_minY + pad;
  int conY = aui_minY + btnH + pad * 2;
  aui_drawtopbar(&aui_win_bg, 0, aui_minY, agw(), btnH + pad * 2);
  /* Init Main Window */
  AWINDOWP hWin = aw(&aui_win_bg);
  /* Top Toolbar */
  imgbtn(hWin, pad, btnY, btnW, btnH, &UI_ICONS[3], "Ctrl+C",   4, 22);
  imgbtn(hWin, pad + btnW, btnY, btnW, btnH, &UI_ICONS[12], "Esc",  4, 21);
  imgbtn(hWin, pad + btnW * 2, btnY, btnW, btnH, &UI_ICONS[14], "Clear",   4, 23);
  imgbtn(hWin, pad + btnW * 3, btnY, btnW, btnH, &UI_ICONS[0],  alang_get("close"), 4, 12);	//-- Close
  /* Create IME2 */
  ACONTROLP imebox = acime2(
                       hWin,
                       0, conY + conH, agw(), imeH,
                       44
                     );
  /* Init Shell */
  AUITERM p;
  p.active  = 0;
  p.pid     = 0;
  p.fd      = 0;
  p.rthread = 0;
  p.drawnow = 0;
  p.basepath = basepath;
  p.console = aconsole(hWin, 0, conY, agw(), conH);
  auiterm_init(&p);
  aconsole_setwindowsize(p.console, p.fd);
  /* Show Window */
  aw_show_ex(hWin, 2, 0, p.console);
  /* Window Messages Loop */
  byte ondispatch = 1;
  
  do {
    dword msg = aw_dispatch(hWin);
    
    if (!p.active) {
      /* Break if terminal thread terminated */
      break;
    }
    
    switch (aw_gm(msg)) {
      case 12: {
          //-- DONE
          if (p.active) {
            AWMENUITEM mi[2];
            aw_menuset(mi, 0, "yes", 33);
            aw_menuset(mi, 1, "no", 0);
            byte ovr = aw_multiconfirm_ex(hWin,
                                          alang_get("tools.terminal.terminate"),
                                          alang_get("tools.terminal.terminate_info"),
                                          aui_icons(39), mi, 2, 1);
                                          
            if (ovr == 0) {
              ondispatch = 0;
            }
          }
        }
        break;
        
      case 22: {
          //-- Ctrl+C
          auiterm_send(&p, auiterm_ctrl('C'), 0);
        }
        break;
        
      case 21: {
          //-- MORE
          char * bks = "\033";
          write(p.fd, bks, strlen(bks));
        }
        break;
        
      case 23: {
          //-- CLEAR
          char * bks = "\033\bclear\r";
          write(p.fd, bks, strlen(bks));
        }
        break;
        
      case 44: {
          if (p.active) {
            if (aw_gl(msg)) {
              auiterm_send(&p, auiterm_ctrl(aw_gd(msg)), aw_gh(msg));
            }
            else {
              auiterm_send(&p, aw_gd(msg), aw_gh(msg));
            }
          }
        }
        break;
    }
  }
  while (ondispatch);
  
  p.active = 0;
  auiterm_release(&p);
  //-- Window
  aw_destroy(hWin);
}
