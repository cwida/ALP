package org.urbcomp.startdb.compress.elf.compressor32;

public interface ICompressor32 {
    void addValue(float v);
    int getSize();
    byte[] getBytes();
    void close();
    default String getKey() {
        return getClass().getSimpleName();
    }
}
