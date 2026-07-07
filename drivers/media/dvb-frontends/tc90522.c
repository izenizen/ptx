/*
        Toshiba TC90522XBG 2ch OFDM(ISDB-T) + 2ch 8PSK(ISDB-S) demodulator

        Copyright (C) Budi Rachmanto, AreMa Inc. <info@are.ma>

        CHIP            CARDS
        TC90522XBG      Earthsoft PT3, PLEX PX-Q3PE
        TC90532         PLEX PX-BCUD

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.
 */

#include "dvb_math.h"
#include <media/dvb_frontend.h>
#include "tc90522.h"

bool tc90522_r(struct i2c_client *c, u8 slvadr, u8 *buf, u8 len)
{
        struct i2c_msg msg[] = {
                {.addr = 0x80 | c->addr,        .flags = 0,             .buf = &slvadr, .len = 1,},
                {.addr = c->addr,               .flags = I2C_M_RD,      .buf = buf,     .len = len,},
        };
        return i2c_transfer(c->adapter, msg, 2) == 2;
}

bool tc90522_w(struct i2c_client *c, u8 slvadr, u8 dat)
{
        u8 buf[] = {slvadr, dat};
        struct i2c_msg msg[] = {
                {.addr = c->addr,       .flags = 0,     .buf = buf,     .len = 2,},
        };
        return i2c_transfer(c->adapter, msg, 1) == 1;
}

u64 tc90522_n2int(const u8 *data, u8 n)
{
        u32 i, val = 0;

        for (i = 0; i < n; i++) {
                val <<= 8;
                val |= data[i];
        }
        return val;
}

int tc90522_cn_raw(struct dvb_frontend *fe, u16 *raw)
{
        u8      buf[3],
                len     = fe->dtv_property_cache.delivery_system == SYS_ISDBS ? 2 : 3,
                adr     = fe->dtv_property_cache.delivery_system == SYS_ISDBS ? 0xbc : 0x8b;
        bool    ok      = tc90522_r(fe->demodulator_priv, adr, buf, len);
        int     cn      = tc90522_n2int(buf, len);

        if (!ok)
                return -EIO;
        *raw = cn;
        return cn;
}

static s64 cn_s(s64 raw)
{
        s64 x, y;
        raw -= 3000;
        if (raw < 0)
                raw = 0;
        x = int_sqrt(raw << 20);
        y = 16346ll * x - (143410ll << 16);
        y = ((x * y) >> 16) + (502590ll << 16);
        y = ((x * y) >> 16) - (889770ll << 16);
        y = ((x * y) >> 16) + (895650ll << 16);
        y = (588570ll << 16) - ((x * y) >> 16);
        return y < 0 ? 0 : y >> 16;
}

static s64 cn_t(s64 raw)
{
        s64 x, y;
        if (!raw)
                return 0;
        x = (1130911733ll - 10ll * intlog10(raw)) >> 2;
        y = (x >> 2) - (x >> 6) + (x >> 8) + (x >> 9) - (x >> 10) + (x >> 11) + (x >> 12) - (16ll << 22);
        y = ((x * y) >> 22) + (398ll << 22);
        y = ((x * y) >> 22) + (5491ll << 22);
        y = ((x * y) >> 22) + (30965ll << 22);
        return y >> 22;
}

int tc90522_status(struct dvb_frontend *fe, enum fe_status *stat)
{
        enum fe_status                  *festat = i2c_get_clientdata(fe->demodulator_priv);
        struct dtv_frontend_properties  *c      = &fe->dtv_property_cache;
        u16     v16;
        s64     raw     = tc90522_cn_raw(fe, &v16);

        c->cnr.len              = 1;
        c->cnr.stat[0].svalue   = fe->dtv_property_cache.delivery_system == SYS_ISDBS ? cn_s(raw) : cn_t(raw);
        c->cnr.stat[0].scale    = FE_SCALE_DECIBEL;
        *stat = *festat;
        return *festat;
}

enum dvbfe_algo tc90522_get_frontend_algo(struct dvb_frontend *fe)
{
        return DVBFE_ALGO_HW;
}

static u32 fno2kHz(u32 fno)
{
        if (fno < 12)
                return 1049480 + 38360 * fno;
        else if (fno < 23)
                return 1068660 + 38360 * (fno - 12);
        return 0;
}

int tc90522_tune(struct dvb_frontend *fe, bool retune, u32 mode_flags, u32 *delay, enum fe_status *stat)
{
        return 0;
}

int tc90522_probe(struct i2c_client *c)
{
        return 0;
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Toshiba TC90522 Demodulator Driver");
