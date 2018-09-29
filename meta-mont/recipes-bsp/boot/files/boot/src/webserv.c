/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file webserv.c
*   \brief Simple Webpage Server
*   \author Montage
*/

#ifdef CONFIG_SIMPLE_CMD_WEBPAGE
#include <common.h>
#include <lib.h>
#include <netprot.h>

#define htdebug(format, args...)        //printk(format, ##args)
#define LINE_SIZE           300
#define HTTP_PAGE_BUF_SZ    2048
#define lower_case(c)       (c|0x20)

struct upload_state post_state;
struct httpd_state http_state;
struct httpd_state *https = &http_state;

struct page_buffer
{
    int ofs;                    // current position
    char buf[HTTP_PAGE_BUF_SZ];
};

enum
{
    HTS_INIT = 0,
    HTS_CONTENT_TYPE_LEN = 2,
    HTS_CONTENT_BOUNDARY = 3,
    HTS_CONTENT_DATA = 4,
    HTS_CONTENT_DATA_END = 5,
};

struct page_buffer page;

static int find_eol(char **sp, char *endp);
static void httpd_output_func(char *b);
inline static void httpd_page_default();
static inline void httpd_page_init();
static void httpd_page_end(struct tcp_conn_t *tcps);
int httpd_proc_upload(struct tcp_conn_t *tcps, char *tdata, int tlen, int lazy);
int httpd_get_cmd(char **argv, char *tdata, int tlen);
int httpd_proc_cmd(struct tcp_conn_t *tcps, char *tdata, int tlen);
#if defined(CONFIG_CMD_FLASH) && defined(CONFIG_FLASH_CMD_PROGRAM_AFTER_DOWNLOAD_FILE)
extern char downloaded;
#endif

/*!
 * function:
 *
 *  \brief
 *  \param tcps
 *  \param tdata
 *  \param tlen
 *  \return
 */

int httpd_proc_cmd(struct tcp_conn_t *tcps, char *tdata, int tlen)
{
    int argc;
    char *argv[12];

    argc = httpd_get_cmd(argv, tdata, tlen);
    cmd_proc(argc, argv);
    httpd_page_end(tcps);

    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

static inline void httpd_page_init()
{
    page.ofs = 0;
    io_redirect = (int (*)(char *)) httpd_output_func;
}

/*!
 * function:
 *
 *  \brief
 *  \param sp
 *  \param endp
 *  \return
 */
static int find_eol(char **sp, char *endp)
{
    char *cp;
    for (cp = *sp; cp < endp; cp++)
    {
        if (*cp == 0x0d && *(cp + 1) == 0x0a)
        {
            *sp = cp;
            return 1;
        }
    }
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param buf
 *  \return
 */

static void httpd_output_func(char *buf)
{
    char c;
    if ((HTTP_PAGE_BUF_SZ - 40) < page.ofs)
        return;
    for (; (c = *buf++);)
    {
        page.buf[page.ofs++] = c;
        if ('\r' == c)
            page.buf[page.ofs++] = '\r';
    };
}

/*!
 * function:
 *
 *  \brief
 *  \param argv
 *  \param tdata
 *  \param tlen
 *  \return
 */

int httpd_get_cmd(char **argv, char *tdata, int tlen)
{
    char *p, *cmd = tdata + 17; //GET /cli.cgi?cmd=argv[0] argv[1]
    int n = 0, ch, more = 1;

    while (cmd < (tdata + tlen - 8) && more)
    {
        argv[n] = cmd;
      scan:
        while (*cmd != '+' && *cmd != ' ' && *cmd != '%' && *cmd != 0)
            cmd++;
        ch = *cmd & 0xff;
        if (ch == ' ' || ch == 0)
            more = 0;
        else if ('%' == ch)
        {
            sscanf(cmd + 1, "%02x", &ch);
            *cmd++ = ch;
            for (p = cmd; ' ' != *p; p++)
                *p = *(p + 2);
            goto scan;
        }
        *cmd++ = 0;
        if (++n >= 10)
            break;
    }
    for (ch = n; ch < 10; ch++)
        argv[ch] = 0;

#ifdef htdebug
    {
        htdebug("argc=%d\n", n);
        for (ch = 0; ch < n; ch++)
            htdebug("[%s]", argv[ch]);
        htdebug("\n");
    }
#endif
    return n;
}

/*!
 * function:
 *
 *  \brief
 *  \param tcps
 *  \param tdata
 *  \param tlen
 *  \return
 */
extern void fa(void);
extern int verify_image(unsigned int h);
extern int flash_cmd(int argc, char *argv[]);
extern int cmd_rst(int argc, char *argv[]);
int httpd_proc_upload(struct tcp_conn_t *tcps, char *tdata, int tlen, int lazy)
{
    int blen, i, k = 0;
    char buf[300], *line, *cp;
    unsigned int load_addr = bootvars.load_addr;
    if (HTS_CONTENT_DATA == post_state.state)
    {
        if ((post_state.count + tlen) >= post_state.len)
        {
            htdebug("state%d rx=%d\n", post_state.state,
                    post_state.count + tlen);
            post_state.state = HTS_CONTENT_DATA_END;
        }
        memcpy((void *) (load_addr + post_state.current_ofs), (void *) tdata,
               tlen);
        post_state.count += tlen;
        post_state.current_ofs += tlen;

        if (HTS_CONTENT_DATA_END == post_state.state)
        {
            cp = (char *) load_addr;
            blen = strlen(post_state.boundary);
            i = post_state.current_ofs - blen - 6;
            byte_count = post_state.current_ofs;
            for (k = 0; k < 64; k++)
            {
                if (!memcmp(cp + i, "\x0d\x0a--", 4))
                {
                    byte_count = i;
                    break;
                }
                i--;
            }
            printf("rx len=%d (0x%x)\n", byte_count, byte_count);
            //httpd_save_image(load_addr);
            httpd_page_end(tcps);
            post_state.state = 0;
#if defined(CONFIG_CMD_FLASH) && defined(CONFIG_FLASH_CMD_PROGRAM_AFTER_DOWNLOAD_FILE)
            downloaded = 1;
#endif
            // burn firmware after upload successfully
            if (lazy == HTTP_ONE_KEY_UPLOAD)
            {
                fa();
            }
        }
    }
    else if (HTS_CONTENT_DATA > post_state.state)       //parsing header
    {
        htdebug("line%d content-length=%d\n", __LINE__, tlen);
        for (line = cp = tdata;
             (tdata + tlen) > cp && (HTS_CONTENT_DATA_END > post_state.state);
             line = cp)
        {
            if (!find_eol(&cp, tdata + tlen))
                continue;
            *cp = 0;
            if (HTS_CONTENT_TYPE_LEN > post_state.state)
            {
                for (i = 0; (line + i) < cp; i++)
                    buf[i] = lower_case(line[i]);
                buf[i] = 0;

                if (!strncmp("content-type", buf, 12))
                {
                    for (i = (24 + 20); i < 84; i++)    //24+20: "content-type: multipart/form-data; boundary="
                    {
                        if ('-' == line[i])
                            continue;
                        htdebug("state %d content-type boundary=%s\n",
                                post_state.state, line + i);
                        strcpy(post_state.boundary, line + i);
                        post_state.state++;
                        break;
                    }
                }
                else if (!strncmp("content-length", buf, 14))
                {
                    sscanf(buf + 16, "%d", &post_state.len);
                    htdebug("state%d content-length=%d\n", post_state.state,
                            post_state.len);
                    post_state.state++;
                }
            }
            else if (HTS_CONTENT_TYPE_LEN == post_state.state)
            {
                for (i = 0; i < LINE_SIZE; i++)
                    if ('-' != line[i])
                        break;
                if (!strncmp
                    (line + i, post_state.boundary,
                     strlen(post_state.boundary)))
                {
                    htdebug("state%d bound\n", post_state.state);
                    post_state.state = HTS_CONTENT_BOUNDARY;
                    post_state.count =
                        ((unsigned int) tdata + tlen) - (unsigned int) line;
                    htdebug("tlen=%d post_state.count=%d\n", tlen,
                            post_state.count);
                }
            }
            else if (HTS_CONTENT_BOUNDARY == post_state.state)
            {
                if (line == cp)
                {
                    cp += 2;    //sizeof("\r\n");
                    post_state.current_ofs =
                        ((unsigned int) tdata + tlen) - (unsigned int) cp;
                    memcpy((void *) load_addr, (void *) cp,
                           post_state.current_ofs);
                    htdebug("state%d rx:%d\n", post_state.state,
                            post_state.current_ofs);
                    post_state.state = HTS_CONTENT_DATA;
                    break;
                }
            }
            cp += 2;            // sizeof("\r\n");
        }
        return 0;
    }
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param load_addr
 *  \return
 */
int httpd_save_image(unsigned int load_addr)
{
    int ret;
    char *parm[] = { "fav", 0 };
    ret = verify_image(load_addr);
    if (ret > 0)
    {
        ret = flash_cmd(1, &parm[1]);
        if (!ret)
        {
            printf("Upload success!!\r\n");
            cmd_rst(0, 0);
            return 0;
        }
        return -1;
    }
    else
    {
        printf("\n Invalid Image !!\r\n");
        return -1;
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param tcps
 *  \return
 */

int httpd_handle(struct tcp_conn_t *tcps)
{
    int tlen, result;
    char *tdata;

    tdata = (char *) tcps->rxbuf;
    tlen = tcps->rxlen;
    if ((tlen > 0) && https->state == HTTP_NOGET)
    {
        httpd_page_init();
        if (!strncmp("POST /up", &tdata[0], 8))
        {
            https->state = HTTP_UPLOAD;
            post_state.state = 0;
            goto proc;
        }

        if (!strncmp("GET /cli", &tdata[0], 8))
        {
            https->state = HTTP_FUNC;
            goto proc;
        }

        if (!strncmp("POST /lazy", &tdata[0], 10))
        {
            https->state = HTTP_ONE_KEY_UPLOAD;
            post_state.state = 0;
            goto proc;
        }

        httpd_page_default();
        httpd_page_end(tcps);
        return 0;
    }

  proc:
    switch (https->state)
    {
        case HTTP_FUNC:
            result = httpd_proc_cmd(tcps, tdata, tlen);
            break;

        case HTTP_UPLOAD:
            result = httpd_proc_upload(tcps, tdata, tlen, https->state);
            break;

        case HTTP_ONE_KEY_UPLOAD:
            result = httpd_proc_upload(tcps, tdata, tlen, https->state);
            break;

        default:
            if (tcps->rxflag & TH_ACK)
            {
                if (tcps->wait_ack > https->state)
                    https->state = 0;
                else
                {
                    https->dataptr += tcps->wait_ack;
                    https->state -= tcps->wait_ack;
                }

                if (!https->state)
                {               //no data
                    tcps->state = CLOSED;
                    return 0;
                }
            }
            tcps->txlen = https->state;
            tcps->txbuf = (unsigned char *) https->dataptr;
            return 0;
    }

    return result;
}

/*!
 * function:
 *
 *  \brief
 *  \return void
 */

void httpd_init(void)
{
    memset((void *) https, 0, sizeof (*https));
}

/*!
 * function:
 *
 *  \brief
 *  \param tcps
 *  \return void
 */

static void httpd_page_end(struct tcp_conn_t *tcps)
{
    tcps->txlen = page.ofs;
    tcps->txbuf = (unsigned char *) page.buf;
    tcps->state = CLOSED;
    io_redirect = 0;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

inline static void httpd_page_default()
{
    printf("\n<html><body>\n");
    printf
        ("<iframe name=console src=# style='width:800;height:200;border:1px solid white;'></iframe><br>\n");
    printf("<form action=cli.cgi method=get target=console>\n"
           "cmd&gt;<input size=30 name=cmd>\n"
           "<input type=submit value=Send></form>\n");
    printf
        ("<form action=up.htm  method=post target=console encType=multipart/form-data>\n"
         "<input type=file id=\"myFile\" size=20 name=files><input type=submit value=Upload></form>\n");
    printf("<form action=cli.cgi  id=\"my_up\"  method=get target=console>\n"
           "<input type=\"hidden\" name=\"cmd\" value=\"urst\"></form>\n");
    printf
        ("<form action=cli.cgi  id=\"my_form\"  method=get  target=console  encType=multipart/form-data  onClick=\"sendMessage();\">\n"
         "<input type=submit name=\"cmd\" value=\"upgrade\"></form>\n");
    printf
        ("<form action=lazy.htm method=post target=console encType=multipart/form-data>\n"
         "<input type=file id=\"myFile\" size=20 name=files><input type=submit value=One_Key_Upgrade></form>\n");

    printf("\n<script type=\"text/javascript\">\n");
    printf("function submitForm(){\n");
    printf("document.getElementById(\"my_up\").submit();\n");
    printf("}\n");
    printf("function sendMessage(){\n");
    printf("if(document.getElementById(\"my_form\")){\n");
    printf("var x = document.getElementById(\"myFile\")\n");
    printf("var txt = \"\"\n");
    printf("if('files' in x){\n");
    printf("if(x.files.length != 0){\n");
    printf("for(var i = 0; i < x.files.length; i++){\n");
    printf("var file = x.files[i];\n");
    printf("if('size' in file)\n");
    printf("txt += \"size: \" + file.size + \" bytes <br>\"\n");
    printf("}\n");
    printf("}\n");
    printf("}\n");
    printf("var time = (1.05*(file.size))/100\n");
    printf("setTimeout(\"submitForm()\", time); // set timeout\n");
    printf("}\n");
    printf("}\n");
    printf("</script>\n");
    printf("</body></html>\n");
}

#endif
