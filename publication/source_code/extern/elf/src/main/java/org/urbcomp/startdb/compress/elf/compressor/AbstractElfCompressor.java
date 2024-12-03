package org.urbcomp.startdb.compress.elf.compressor;

public abstract class AbstractElfCompressor implements ICompressor {
    // αlog_2(10) for look-up
    private final static int[] f =
                    new int[] {0, 4, 7, 10, 14, 17, 20, 24, 27, 30, 34, 37, 40, 44, 47, 50, 54, 57,
                                    60, 64, 67};

    private final static double[] map10iP =
                    new double[] {1.0, 1.0E1, 1.0E2, 1.0E3, 1.0E4, 1.0E5, 1.0E6, 1.0E7,
                                    1.0E8, 1.0E9, 1.0E10, 1.0E11, 1.0E12, 1.0E13, 1.0E14,
                                    1.0E15, 1.0E16, 1.0E17, 1.0E18, 1.0E19, 1.0E20};
    private final static double LOG_2_10 = Math.log(10) / Math.log(2);

    private int size = 0;

    public void addValue(double v) {
        long vLong = Double.doubleToRawLongBits(v);
        long vPrimeLong;

        if (v == 0.0 || Double.isInfinite(v)) {
            size += writeBit(false);
            vPrimeLong = vLong;
        } else if (Double.isNaN(v)) {
            size += writeBit(false);
            vPrimeLong = 0x7ff8000000000000L;
        } else {
            int[] alphaAndBetaStar = getAlphaAndBetaStar(v);
            int e = ((int) (vLong >> 52)) & 0x7ff;
            int gAlpha = getFAlpha(alphaAndBetaStar[0]) + e - 1023;
            int eraseBits = 52 - gAlpha;
            long mask = 0xffffffffffffffffL << eraseBits;
            long delta = (~mask) & vLong;
            if (alphaAndBetaStar[1] < 16 && delta != 0 && eraseBits > 4) {
                size += writeInt(alphaAndBetaStar[1] | 0x10, 5);
                vPrimeLong = mask & vLong;
            } else {
                size += writeBit(false);
                vPrimeLong = vLong;
            }
        }
        size += xorCompress(vPrimeLong);
    }

    public int getSize() {
        return size;
    }

    protected abstract int writeInt(int n, int len);

    protected abstract int writeBit(boolean bit);

    protected abstract int xorCompress(long vPrimeLong);

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

    private static int[] getAlphaAndBetaStar(double v) {
        if (v < 0) {
            v = -v;
        }
        int[] alphaAndBetaStar = new int[2];
        double log10v = Math.log10(v);
        int sp = (int) Math.floor(log10v);
        int beta = getSignificantCount(v, sp);
        alphaAndBetaStar[0] = beta - sp - 1;
        alphaAndBetaStar[1] = (v < 1 && sp == log10v) ? 0 : beta;
        return alphaAndBetaStar;
    }

    private static int getSignificantCount(double v, int sp) {
        int i;
        if(sp >= 0) {
            i = 1;
        } else {
            i = -sp;
        }
        double temp = v * get10iP(i);
        while ((long) temp != temp) {
            i++;
            temp = v * get10iP(i);
        }
        // There are some bugs for those with high significand, i.e., 0.23911204406033099
        // So we should further check
        if (temp / get10iP(i) != v) {
            return 17;
        } else {
            return sp + i + 1;
        }
    }

    private static double get10iP(int i) {
        if (i <= 0) {
            throw new IllegalArgumentException("The argument should be greater than 0");
        }
        if (i >= map10iP.length) {
            return Double.parseDouble("1.0E" + i);
        } else {
            return map10iP[i];
        }
    }
}
