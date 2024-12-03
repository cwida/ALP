package org.urbcomp.startdb.compress.elf.xordecompressor;

import gr.aueb.delorean.chimp.InputBitStream;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class ElfXORDecompressor32 {
    private int storedVal = 0;
    private int storedLeadingZeros = Integer.MAX_VALUE;
    private int storedTrailingZeros = Integer.MAX_VALUE;
    private boolean first = true;
    private boolean endOfStream = false;

    private final InputBitStream in;

    private final static int END_SIGN = Float.floatToIntBits(Float.NaN);

    private final static short[] leadingRepresentation = {0, 6, 10, 12, 14, 16, 18, 20};

    public ElfXORDecompressor32(byte[] bs) {
        in = new InputBitStream(bs);
    }

    public List<Float> getValues() {
        List<Float> list = new ArrayList<>(1024);
        Float value = readValue();
        while (value != null) {
            list.add(value);
            value = readValue();
        }
        return list;
    }

    public InputBitStream getInputStream() {
        return in;
    }

    /**
     * Returns the next pair in the time series, if available.
     *
     * @return Pair if there's next value, null if series is done.
     */
    public Float readValue() {
        try {
            next();
        } catch (IOException e) {
            throw new RuntimeException(e.getMessage());
        }
        if (endOfStream) {
            return null;
        }
        return Float.intBitsToFloat(storedVal);
    }

    private void next() throws IOException {
        if (first) {
            first = false;
            int trailingZeros = in.readInt(6);
            storedVal = in.readInt(32 - trailingZeros) << trailingZeros;
            if (storedVal == END_SIGN) {
                endOfStream = true;
            }
        } else {
            nextValue();
        }
    }

    private void nextValue() throws IOException {
        int value;
        int centerBits, leadAndCenter;
        int flag = in.readInt(2);
        switch (flag) {
            case 3:
                // case 11
                leadAndCenter = in.readInt(8);
                storedLeadingZeros = leadingRepresentation[leadAndCenter >>> 5];
                centerBits = leadAndCenter & 0x1f;
                if(centerBits == 0) {
                    centerBits = 32;
                }
                storedTrailingZeros = 32 - storedLeadingZeros - centerBits;
                value = in.readInt(centerBits) << storedTrailingZeros;
                value = storedVal ^ value;
                if (value == END_SIGN) {
                    endOfStream = true;
                } else {
                    storedVal = value;
                }
                break;
            case 2:
                // case 10
                leadAndCenter = in.readInt(6);
                storedLeadingZeros = leadingRepresentation[leadAndCenter >>> 3];
                centerBits = leadAndCenter & 0x7;
                if(centerBits == 0) {
                    centerBits = 8;
                }
                storedTrailingZeros = 32 - storedLeadingZeros - centerBits;
                value = in.readInt(centerBits) << storedTrailingZeros;
                value = storedVal ^ value;
                if (value == END_SIGN) {
                    endOfStream = true;
                } else {
                    storedVal = value;
                }
                break;
            case 1:
                // case 01, we do nothing, the same value as before
                break;
            default:
                // case 00
                centerBits = 32 - storedLeadingZeros - storedTrailingZeros;
                value = in.readInt(centerBits) << storedTrailingZeros;
                value = storedVal ^ value;
                if (value == END_SIGN) {
                    endOfStream = true;
                } else {
                    storedVal = value;
                }
                break;
        }
    }
}
