package org.urbcomp.startdb.compress.elf.compressor;

public interface ICompressor {
    void addValue(double v);
    int getSize();
    byte[] getBytes();
    void close();
    default String getKey() {
        return getClass().getSimpleName();
    }
}
