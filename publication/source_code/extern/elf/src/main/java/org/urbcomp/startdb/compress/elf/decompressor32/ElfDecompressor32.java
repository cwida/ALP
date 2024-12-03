package org.urbcomp.startdb.compress.elf.decompressor32;

import gr.aueb.delorean.chimp.InputBitStream;
import org.urbcomp.startdb.compress.elf.xordecompressor.ElfXORDecompressor32;

import java.io.IOException;

public class ElfDecompressor32 extends AbstractElfDecompressor32{
    private final ElfXORDecompressor32 xorDecompressor32;

    public ElfDecompressor32(byte[] bytes) {
        xorDecompressor32 = new ElfXORDecompressor32(bytes);
    }

    @Override protected Float xorDecompress() {
        return xorDecompressor32.readValue();
    }

    @Override protected int readInt(int len) {
        InputBitStream in = xorDecompressor32.getInputStream();
        try {
            return in.readInt(len);
        } catch (IOException e) {
            throw new RuntimeException("IO error: " + e.getMessage());
        }
    }
}
