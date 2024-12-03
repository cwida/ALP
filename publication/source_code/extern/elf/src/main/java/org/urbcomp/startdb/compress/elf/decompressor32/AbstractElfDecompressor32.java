package org.urbcomp.startdb.compress.elf.decompressor32;


import java.util.ArrayList;
import java.util.List;

public abstract class AbstractElfDecompressor32 implements IDecompressor32 {

    private final static float[] map10iN =
            new float[] {1.0f, 1.0E-1f, 1.0E-2f, 1.0E-3f, 1.0E-4f, 1.0E-5f, 1.0E-6f, 1.0E-7f,
                    1.0E-8f, 1.0E-9f, 1.0E-10f, 1.0E-11f, 1.0E-12f, 1.0E-13f, 1.0E-14f,
                    1.0E-15f, 1.0E-16f, 1.0E-17f, 1.0E-18f, 1.0E-19f, 1.0E-20f};
    private final static float[] map10iP =
            new float[] {1.0f, 1.0E1f, 1.0E2f, 1.0E3f, 1.0E4f, 1.0E5f, 1.0E6f, 1.0E7f,
                    1.0E8f, 1.0E9f, 1.0E10f, 1.0E11f, 1.0E12f, 1.0E13f, 1.0E14f,
                    1.0E15f, 1.0E16f, 1.0E17f, 1.0E18f, 1.0E19f, 1.0E20f};

    public List<Float> decompress() {
        List<Float> values = new ArrayList<>(1024);
        Float value;
        while ((value = nextValue()) != null) {
            values.add(value);
        }
        return values;
    }

    private Float nextValue() {
        int flag = readInt(1);

        Float v;
        if (flag == 0) {
            v = xorDecompress();
        } else {
            int betaStar = readInt(3);
            Float vPrime = xorDecompress();
            int sp = (int) Math.floor(Math.log10(Math.abs(vPrime)));
            if (betaStar == 0) {
                v = get10iN(-sp - 1);
                if (vPrime < 0) {
                    v = -v;
                }
            } else {
                int alpha = betaStar - sp - 1;
                v = roundUp(vPrime, alpha);
            }
        }
        return v;
    }

    protected abstract Float xorDecompress();

    protected abstract int readInt(int len);

    private static float get10iN(int i) {
        if (i <= 0) {
            throw new IllegalArgumentException("The argument should be greater than 0");
        }
        if (i >= map10iN.length) {
            return Float.parseFloat("1.0E-" + i);
        } else {
            return map10iN[i];
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

    private static float roundUp(float v, int alpha) {
        float scale = get10iP(alpha);
        if (v < 0) {
            return (float) (Math.floor(v * scale) / scale);
        } else {
            return (float) (Math.ceil(v * scale) / scale);
        }
    }
}
