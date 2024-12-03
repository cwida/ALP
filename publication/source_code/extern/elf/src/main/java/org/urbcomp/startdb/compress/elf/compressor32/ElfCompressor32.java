package org.urbcomp.startdb.compress.elf.compressor32;

import gr.aueb.delorean.chimp.OutputBitStream;
import org.urbcomp.startdb.compress.elf.xorcompressor.ElfXORCompressor32;

public class ElfCompressor32 extends AbstractElfCompressor32 {
    private final ElfXORCompressor32 xorCompressor32;

    public ElfCompressor32() {
        xorCompressor32 = new ElfXORCompressor32();
    }

    @Override protected int writeInt(int n, int len) {
        OutputBitStream os = xorCompressor32.getOutputStream();
        os.writeInt(n, len);
        return len;
    }

    @Override protected int writeBit(boolean bit) {
        OutputBitStream os = xorCompressor32.getOutputStream();
        os.writeBit(bit);
        return 1;
    }

    @Override protected int xorCompress(int vPrimeInt) {
        return xorCompressor32.addValue(vPrimeInt);
    }

    @Override public byte[] getBytes() {
        return xorCompressor32.getOut();
    }

    @Override public void close() {
        // we write one more bit here, for marking an end of the stream.
        writeBit(false);
        xorCompressor32.close();
    }
}
