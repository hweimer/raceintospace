// This file is something to do with processing audio files

#include "pace.h"

#include <cassert>
#include <cctype>

#include "display/graphics.h"
#include "display/surface.h"

#include "Buzz_inc.h"
#include "utils.h"
#include "game_main.h"
#include "sdlhelper.h"
#include "gr.h"
#include "mmfile.h"


void randomize(void);
void SMove(void *p, int x, int y);
void LMove(void *p);
double get_time(void);


LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)

void seq_init(void);


char DoModem(int sel)
{
    NOTICE1("DoModem not implemented");
    return (0);
}

char MPrefs(char mode)
{
    return (0);
}

int put_serial(unsigned char n)
{
    return 0;
}
void MesCenter(void) {}

char *letter_dat;

void OpenEmUp(void)
{
    randomize();
    letter_dat = slurp_gamedat("letter.dat");
}

int PCX_D(const char *src_raw, char *dest_raw, unsigned src_size)
{
    const char *src = (const char *)src_raw;
    char *dest = (char *)dest_raw;
    char num;
    char *orig_dest = dest;

    do {
        if ((*src & 0xc0) == 0xc0) {
            num = *(src++) & 0x3f;
            src_size--;

            while ((num--) > 0) {
                *(dest++) = *src;
            }

            src++;
            src_size--;
        } else {
            (*dest++) = *(src++);
            src_size--;
        }
    }  while (src_size);

    return (dest - orig_dest);
}

/* Run-length Encoding Decompression algorithm.
 *
 * Expands a data sequence compressed using the RLEC() function.
 *
 * \param src_raw   Compressed input buffer.
 * \param dest_raw  Buffer for decompressed ouput.
 * \param src_size  Length of the compressed file, in bytes.
 * \return  Size of the decompressed data in bytes.
 */
int RLED(const char *src_raw, char *dest_raw, unsigned int src_size)
{
    const signed char *src = (const signed char *)src_raw;
    signed char *dest = (signed char *)dest_raw;
    unsigned int used;
    int count, val;
    int i;

    used = 0;

    while (used < src_size) {
        count = src[used++];

        if (count < 0) {
            count = -count + 1;
            val = src[used++];

            for (i = 0; i < count; i++) {
                *dest++ = val;
            }
        } else {
            count++;

            for (i = 0; i < count; i++) {
                *dest++ = src[used++];
            }
        }
    }

    return ((char *)dest - (char *)dest_raw);
}

int RLED_img(const char *src_raw, char *dest_raw, unsigned int src_size,
             int w, int h)
{
    const signed char *src = (const signed char *)src_raw;
    signed char *dest;
    unsigned int used;
    int count, val;
    int total;
    signed char buf[128 * 1024];
    int row;

    dest = buf;

    used = 0;

    while (used < src_size) {
        count = src[used++];

        if (count < 0) {
            count = -count + 1;
            val = src[used++];
            memset(dest, val, count);
            dest += count;
        } else {
            count++;
            memcpy(dest, &src[used], count);
            used += count;
            dest += count;
        }
    }

    total = dest - buf;

    if (total < w * h + h) {
        memcpy(dest_raw, buf, w * h);
        return (w * h);
    }

    dest = (signed char *)dest_raw;

    for (row = 0; row < h; row++) {
        memcpy(dest, &buf[row * (w + 1)], w);
        dest += w;
    }

    return (w * h);
}

/**
 * Original sources say:
 * @param wh = 0 - fade colors from 0 to val,
 *           = 1 - fade colors from val to 3*256,
 *           = 2 - fade all colors
 * @param palx pointer to palette
 * @param steps
 * @param val pivot index in palette array
 * @param mode if mode == 1 then preserve non-faded colors, else make black
 */
void FadeIn(char wh, int steps, int val, char mode)
{
    return;
    int from = 0;
    int to = 256;

    if (wh == 0) {
        to = val;
    } else if (wh == 1) {
        from = val;
    } else {
        assert(wh == 2);
    }

    av_set_fading(AV_FADE_IN, from, to, steps, !!mode);
}

void FadeOut(char wh, int steps, int val, char mode)
{
    return;
    int from = 0;
    int to = 256;

    if (wh == 0) {
        to = val;
    } else if (wh == 1) {
        from = val;
    } else {
        assert(wh == 2);
    }

    av_set_fading(AV_FADE_OUT, from, to, steps, !!mode);
}

void delay(int millisecs)
{
    idle_loop_secs(millisecs / 1000.0);
}

void bzdelay(int ticks)
{
    idle_loop_secs(ticks / 100.0);
}

int brandom(int limit)
{
    if (limit == 0) {
        return (0);
    }

    return (int)(limit * (rand() / (RAND_MAX + 1.0)));
}

void StopAudio(char mode)
{
    av_silence(AV_SOUND_CHANNEL);
}

void CloseEmUp(unsigned char error, unsigned int value)
{
    /* DEBUG */ /* fprintf (stderr, "CloseEmUp()\n"); */
    exit(EXIT_SUCCESS);
}

void SMove(void *p, int x, int y)
{
    display::LegacySurface local(160, 100);
    memcpy(local.pixels(), p, 160 * 100);
    local.copyTo(display::graphics.legacyScreen(), x, y);
}

void LMove(void *p)
{
    display::graphics.screen()->clear();

    display::LegacySurface local(160, 100);
    memcpy(local.pixels(), p, 160 * 100);
    local.copyTo(display::graphics.legacyScreen(), 320 / 4, 200 / 4);
}

void randomize(void)
{
    srand(0 * get_time() * 1000);
}

/** do nothing for a few seconds.
 *
 * The function will wait a number of seconds but will call av_block() in the meantime.
 *
 * \param secs Number of seconds to wait.
 */
void idle_loop_secs(double secs)
{
    double start;

    gr_sync();

    start = get_time();

    while (1) {
        av_block();

        if (get_time() - start >= secs) {
            break;
        }
    }
}

/** wait a number of ticks
 *
 * \param ticks Number of ticks to wait.
 */
void idle_loop(int ticks)
{
    idle_loop_secs(ticks / 2000.0);
}

char *soundbuf;
size_t soundbuf_size = 0;
size_t soundbuf_used = 0;
struct audio_chunk news_chunk;

ssize_t load_audio_file(const char *name, char **data, size_t *size)
{
    mm_file mf;
    unsigned channels, rate;
    const size_t def_size = 16 * 1024;
    size_t offset = 0;
    ssize_t read = 0;
    double start = get_time();

    /* make compiler happy */
    start *= 1.0;

    assert(name);
    assert(data);
    assert(size);

    if (mm_open_fp(&mf, sOpen(name, "rb", FT_AUDIO)) < 0) {
        return -1;
    }

    if (mm_audio_info(&mf, &channels, &rate) < 0) {
        CWARNING3(audio, "no audio data in file `%s'", name);
        mm_close(&mf);
        return -1;
    }

    if (channels != 1 || rate != 11025) {
        CERROR3(audio, "file `%s' should be mono, 11025Hz", name);
        mm_close(&mf);
        return -1;
    }

    if (!*data) {
        *data = (char *)xmalloc(*size = def_size);
    }

    while (0 < (read = mm_decode_audio(&mf,
                                       *data + offset, *size - offset))) {
        offset += read;

        if (*size <= offset) {
            *data = (char *)xrealloc(*data, *size *= 2);
        }
    }

    mm_close(&mf);

    CDEBUG4(audio, "loading file `%s' took %5.4f seconds",
            name, get_time() - start);

    return offset;
}

void NGetVoice(char plr, char val)
{
    char fname[100];
    ssize_t bytes = 0;

    snprintf(fname, sizeof(fname), "%s_%03d.ogg",
             (plr ? "sov" : "usa"), val);
    bytes = load_audio_file(fname, &soundbuf, &soundbuf_size);
    soundbuf_used = (bytes > 0) ? bytes : 0;
}

void PlayVoice(void)
{
    if (!soundbuf_used) {
        return;
    }

    news_chunk.data = soundbuf;
    news_chunk.size = soundbuf_used;
    news_chunk.next = NULL;
    play(&news_chunk, AV_SOUND_CHANNEL);
}

void KillVoice(void)
{
    av_silence(AV_SOUND_CHANNEL);
}

void StopVoice(void)
{
    av_silence(AV_SOUND_CHANNEL);
}

void PlayAudio(const char *name, char mode)
{
    ssize_t bytes = 0;
    bytes = load_audio_file(name, &soundbuf, &soundbuf_size);
    soundbuf_used = (bytes > 0) ? bytes : 0;
    PlayVoice();
}

int getch(void)
{
    int c;

    while (1) {
        av_block();

        if ((c = bioskey(0)) != 0) {
            return (c);
        }
    }
}

void play_audio(std::string str, int mode)
{
    char filename[40];
    ssize_t size;

    snprintf(filename, sizeof(filename), "%s.ogg", str.c_str());

    CINFO3(audio, "play sound file `%s'", filename);
    size = load_audio_file(filename, &soundbuf, &soundbuf_size);
    soundbuf_used = (size > 0) ? size : 0;
    PlayVoice();
}
