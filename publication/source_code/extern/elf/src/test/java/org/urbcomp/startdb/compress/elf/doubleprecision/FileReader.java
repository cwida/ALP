package org.urbcomp.startdb.compress.elf.doubleprecision;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Paths;

public class FileReader {
    public static final int DEFAULT_BLOCK_SIZE = 1024;
    private final DataInputStream bufferedReader;
    private final int blockSize;

    public FileReader(String filePath, int blockSize) throws IOException {
        this.bufferedReader = new DataInputStream(new BufferedInputStream(new FileInputStream(filePath)));
        this.blockSize = blockSize;
    }

    public FileReader(String filePath) throws IOException {
        this(filePath, DEFAULT_BLOCK_SIZE);
    }

    public double readDouble() throws IOException  {
        byte[] bytes = new byte[8];
        this.bufferedReader.readFully(bytes); // Ensure exactly 8 bytes are read
        // Convert the bytes to a double using little-endian order
        return ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN).getDouble();
    }


    public double[] nextBlock() {
        double[] values = new double[DEFAULT_BLOCK_SIZE];
        int counter = 0;
        try {
            while (bufferedReader.available() >= Double.BYTES) {
                try {
                    values[counter++] = this.readDouble();
                    if (counter == blockSize) {
                        return values;
                    }
                } catch (NumberFormatException | ArrayIndexOutOfBoundsException ignored) {
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    private String getSubString(String str, int beta) {
        if (str.charAt(0) == '-') {
            beta++;
            if (str.charAt(1) == '0') {
                beta++;
            }
        } else if (str.charAt(0) == '0') {
            beta++;
        }
        beta++;
        if (str.length() <= beta) {
            return str;
        } else {
            return str.substring(0, beta);
        }
    }
}
