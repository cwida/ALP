package org.urbcomp.startdb.compress.elf.compressor32;


public abstract class AbstractElfCompressor32 implements ICompressor32 {
    // αlog_2(10) for look-up
    private final static int[] f =
            new int[]{0, 4, 7, 10, 14, 17, 20, 24, 27, 30, 34, 37, 40, 44, 47, 50, 54, 57,
                    60, 64, 67};

    private final static float[] map10iP =
            new float[]{1.0f, 1.0E1f, 1.0E2f, 1.0E3f, 1.0E4f, 1.0E5f, 1.0E6f, 1.0E7f,
                    1.0E8f, 1.0E9f, 1.0E10f, 1.0E11f, 1.0E12f, 1.0E13f, 1.0E14f,
                    1.0E15f, 1.0E16f, 1.0E17f, 1.0E18f, 1.0E19f, 1.0E20f};
    private final static double LOG_2_10 = Math.log(10) / Math.log(2);

    private int size = 0;

    public void addValue(float v) {
        int vInt = Float.floatToRawIntBits(v);
        int vPrimeInt;

        if (v == 0.0 || Float.isInfinite(v)) {
            size += writeBit(false);
            vPrimeInt = vInt;
        } else if (Float.isNaN(v)) {
            size += writeBit(false);
            vPrimeInt = 0x7fc00000;
        } else {
            int[] alphaAndBetaStar = getAlphaAndBetaStar(v);
            int e = (vInt >> 23) & 0xff;
            int gAlpha = getFAlpha(alphaAndBetaStar[0]) + e - 127;
            int eraseBits = 23 - gAlpha;
            int mask = 0xffffffff << eraseBits;
            int delta = (~mask) & vInt;
            if (delta != 0 && eraseBits > 3) {
                size += writeInt(alphaAndBetaStar[1] | 0x8, 4);
                vPrimeInt = mask & vInt;
            } else {
                size += writeBit(false);
                vPrimeInt = vInt;
            }
        }
        size += xorCompress(vPrimeInt);
    }

    public int getSize() {
        return size;
    }

    protected abstract int writeInt(int n, int len);

    protected abstract int writeBit(boolean bit);

    protected abstract int xorCompress(int vPrimeInt);

    private static int getFAlpha(int alpha) {
        if (alpha <= 0) {
            throw new IllegalArgumentException("The argument should be greater than 0");
        }
        if (alpha >= f.length) {
            return (int) Math.ceil(alpha * LOG_2_10);
        } else {
            return f[alpha];
        }
    }

    private static int[] getAlphaAndBetaStar(float v) {
        if (v < 0) {
            v = -v;
        }
        int[] alphaAndBetaStar = new int[2];
        float log10v = (float) Math.log10(v);
        int sp = (int) Math.floor(log10v);
        int beta = getSignificantCount(v, sp);
        alphaAndBetaStar[0] = beta - sp - 1;
        alphaAndBetaStar[1] = (v < 1 && sp == log10v) ? 0 : beta;
        return alphaAndBetaStar;
    }

    private static int getSignificantCount(float v, int sp) {
        int i;
        if (sp >= 0) {
            i = 1;
        } else {
            i = -sp;
        }
        float temp = v * get10iP(i);
        while ((int) temp != temp) {
            i++;
            temp = v * get10iP(i);
        }
        // There are some bugs for those with high significand, i.e., 0.23911204406033099
        // So we should further check
        if (temp / get10iP(i) != v) {
            return 8;
        } else {
            return sp + i + 1;
        }
    }

    private static float get10iP(int i) {
        if (i <= 0) {
            throw new IllegalArgumentException("The argument should be greater than 0");
        }
        if (i >= map10iP.length) {
            return Float.parseFloat("1.0E" + i);
        } else {
            return map10iP[i];
        }
    }
}
