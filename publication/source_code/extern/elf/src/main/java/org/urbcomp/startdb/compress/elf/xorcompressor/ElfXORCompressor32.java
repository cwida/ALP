package org.urbcomp.startdb.compress.elf.xorcompressor;

import gr.aueb.delorean.chimp.OutputBitStream;

public class ElfXORCompressor32 {
    private int storedLeadingZeros = Integer.MAX_VALUE;

    private int storedTrailingZeros = Integer.MAX_VALUE;
    private int storedVal = 0;
    private boolean first = true;
    private int size;
    private final static int END_SIGN = Float.floatToIntBits(Float.NaN);

    public final static short[] leadingRepresentation = {
            0, 0, 0, 0, 0, 0, 1, 1,
            1, 1, 2, 2, 3, 3, 4, 4,
            5, 5, 6, 6, 7, 7, 7, 7,
            7, 7, 7, 7, 7, 7, 7, 7
    };

    public final static short[] leadingRound = {
            0, 0, 0, 0, 0, 0, 6, 6,
            6, 6, 10, 10, 12, 12, 14, 14,
            16, 16, 18, 18, 20, 20, 20, 20,
            20, 20, 20, 20, 20, 20, 20, 20
    };

    private final OutputBitStream out;

    public ElfXORCompressor32() {
        out = new OutputBitStream(
                new byte[6000]);  // for elf, we need one more bit for each at the worst case
        size = 0;
    }

    public OutputBitStream getOutputStream() {
        return this.out;
    }

    /**
     * Adds a new long value to the series. Note, values must be inserted in order.
     *
     * @param value next floating point value in the series
     */
    public int addValue(int value) {
        if (first) {
            return writeFirst(value);
        } else {
            return compressValue(value);
        }
    }

    /**
     * Adds a new double value to the series. Note, values must be inserted in order.
     *
     * @param value next floating point value in the series
     */
    public int addValue(float value) {
        if (first) {
            return writeFirst(Float.floatToRawIntBits(value));
        } else {
            return compressValue(Float.floatToRawIntBits(value));
        }
    }

    private int writeFirst(int value) {
        first = false;
        storedVal = value;
        int trailingZeros = Integer.numberOfTrailingZeros(value);
        out.writeInt(trailingZeros, 6);
        out.writeLong(storedVal >>> trailingZeros, 32 - trailingZeros);

        size += 38 - trailingZeros;
        return 38 - trailingZeros;
    }

    /**
     * Closes the block and writes the remaining stuff to the BitOutput.
     */
    public void close() {
        addValue(END_SIGN);
        out.writeBit(false);
        out.flush();
    }

    private int compressValue(int value) {
        int thisSize = 0;
        int xor = storedVal ^ value;

        if (xor == 0) {
            // case 01
            out.writeInt(1, 2);
            size += 2;
            thisSize += 2;
        } else {
            int leadingZeros = leadingRound[Integer.numberOfLeadingZeros(xor)];
            int trailingZeros = Integer.numberOfTrailingZeros(xor);

            if (leadingZeros == storedLeadingZeros && trailingZeros >= storedTrailingZeros) {
                // case 00
                int centerBits = 32 - storedLeadingZeros - storedTrailingZeros;
                int len = 2 + centerBits;
                if(len > 32) {
                    out.writeInt(0, 2);
                    out.writeInt(xor >>> storedTrailingZeros, centerBits);
                } else {
                    out.writeInt(xor >>> storedTrailingZeros, len);
                }

                size += len;
                thisSize += len;
            } else {
                storedLeadingZeros = leadingZeros;
                storedTrailingZeros = trailingZeros;
                int centerBits = 32 - storedLeadingZeros - storedTrailingZeros;
                if (centerBits <= 8) {
                    // case 10
                    out.writeInt((((0x2 << 3) | leadingRepresentation[storedLeadingZeros]) << 3) | (centerBits & 0x7), 8);
                    out.writeInt(xor >>> storedTrailingZeros, centerBits);

                    size += 8 + centerBits;
                    thisSize += 8 + centerBits;
                } else {
                    // case 11
                    out.writeInt((((0x3 << 3) | leadingRepresentation[storedLeadingZeros]) << 5) | (centerBits & 0x1f), 10);
                    out.writeInt(xor >>> storedTrailingZeros, centerBits);

                    size += 10 + centerBits;
                    thisSize += 10 + centerBits;
                }
            }

            storedVal = value;
        }

        return thisSize;
    }

    public int getSize() {
        return size;
    }

    public byte[] getOut() {
        return out.getBuffer();
    }
}
