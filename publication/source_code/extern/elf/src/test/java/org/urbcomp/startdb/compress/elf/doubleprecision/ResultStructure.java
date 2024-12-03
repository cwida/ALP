package org.urbcomp.startdb.compress.elf.doubleprecision;

import sun.misc.DoubleConsts;

import java.util.Comparator;
import java.util.List;

import java.util.HashMap;
import java.util.Map;

public class ResultStructure {
    private String filename;
    private String compressorName;
    private double compressorRatio;
    private double compressionTime;
    private double maxCompressTime;
    private double minCompressTime;
    private double mediaCompressTime;
    private double decompressionTime;
    private double maxDecompressTime;
    private double minDecompressTime;
    private double mediaDecompressTime;

    private static final Map<String, String> datasetNames = new HashMap<>();
    static {
        datasetNames.put("city_temperature_f", "City-Temp");
        datasetNames.put("stocks_uk", "Stocks-UK");
        datasetNames.put("stocks_usa_c", "Stocks-USA");
        datasetNames.put("stocks_de", "Stocks-DE");
        datasetNames.put("neon_bio_temp_c", "IR-bio-temp");
        datasetNames.put("neon_wind_dir", "Wind-dir");
        datasetNames.put("neon_pm10_dust", "PM10-dust");
        datasetNames.put("neon_dew_point_temp", "Dew-Point-Temp");
        datasetNames.put("neon_air_pressure", "Air-Pressure");
        datasetNames.put("basel_wind_f", "Basel-wind");
        datasetNames.put("basel_temp_f", "Basel-temp");
        datasetNames.put("bitcoin_f", "Bitcoin-price");
        datasetNames.put("bitcoin_f_f", "Bitcoin-price");
        datasetNames.put("bird_migration_f", "Bird-migration");
        datasetNames.put("air_sensor_f", "Air-sensor");
        datasetNames.put("food_prices", "Food-prices");
        datasetNames.put("poi_lat", "POI-lat");
        datasetNames.put("poi_lon", "POI-lon");
        datasetNames.put("bitcoin_transactions_f", "Blockchain-tr");
        datasetNames.put("ssd_hdd_benchmarks_f", "SD-bench");
        datasetNames.put("gov10", "Gov/10");
        datasetNames.put("gov26", "Gov/26");
        datasetNames.put("gov30", "Gov/30");
        datasetNames.put("gov31", "Gov/31");
        datasetNames.put("gov40", "Gov/40");
        datasetNames.put("medicare1", "Medicare/1");
        datasetNames.put("medicare9", "Medicare/9");
        datasetNames.put("nyc29", "NYC/29");
        datasetNames.put("cms1", "CMS/1");
        datasetNames.put("cms9", "CMS/9");
        datasetNames.put("cms25", "CMS/25");
        datasetNames.put("arade4", "Arade/4");
    }

    private static final Map<String, String> datasetNamesCR = new HashMap<>();
    static {
        datasetNamesCR.put("city_temperature_f", "City-Temp");
        datasetNamesCR.put("stocks_uk", "Stocks-UK");
        datasetNamesCR.put("stocks_usa_c", "Stocks-USA");
        datasetNamesCR.put("stocks_de", "Stocks-DE");
        datasetNamesCR.put("neon_bio_temp_c", "Bio-Temp");
        datasetNamesCR.put("neon_wind_dir", "Wind-dir");
        datasetNamesCR.put("neon_pm10_dust", "PM10-dust");
        datasetNamesCR.put("neon_dew_point_temp", "Dew-Temp");
        datasetNamesCR.put("neon_air_pressure", "Air-Pressure");
        datasetNamesCR.put("basel_wind_f", "Basel-Wind");
        datasetNamesCR.put("basel_temp_f", "Basel-Temp");
        datasetNamesCR.put("bitcoin_f", "Btc-Price");
        datasetNamesCR.put("bitcoin_f_f", "Btc-Price");
        datasetNamesCR.put("bird_migration_f", "Bird-Mig");
        datasetNamesCR.put("air_sensor_f", "Air-sensor");
        datasetNamesCR.put("food_prices", "Food-prices");
        datasetNamesCR.put("poi_lat", "POI-lat");
        datasetNamesCR.put("poi_lon", "POI-lon");
        datasetNamesCR.put("bitcoin_transactions_f", "Blockchain");
        datasetNamesCR.put("ssd_hdd_benchmarks_f", "SD-bench");
        datasetNamesCR.put("gov10", "Gov/10");
        datasetNamesCR.put("gov26", "Gov/26");
        datasetNamesCR.put("gov30", "Gov/30");
        datasetNamesCR.put("gov31", "Gov/31");
        datasetNamesCR.put("gov40", "Gov/40");
        datasetNamesCR.put("medicare1", "Medicare/1");
        datasetNamesCR.put("medicare9", "Medicare/9");
        datasetNamesCR.put("nyc29", "NYC/29");
        datasetNamesCR.put("cms1", "CMS/1");
        datasetNamesCR.put("cms9", "CMS/9");
        datasetNamesCR.put("cms25", "CMS/25");
        datasetNamesCR.put("arade4", "Arade/4");
    }

    public static String mapDatasetName(String key) {
        return datasetNames.getOrDefault(key, null);
    }

    public static String mapDatasetNameCR(String key) {
        return datasetNamesCR.getOrDefault(key, null);
    }

    public ResultStructure(String filename, String compressorName, double compressorRatio, double compressionTime, double maxCompressTime, double minCompressTime, double mediaCompressTime, double decompressionTime, double maxDecompressTime, double minDecompressTime, double mediaDecompressTime) {
        this.filename = filename;
        this.compressorName = compressorName;
        this.compressorRatio = compressorRatio;
        this.compressionTime = compressionTime;
        this.maxCompressTime = maxCompressTime;
        this.minCompressTime = minCompressTime;
        this.mediaCompressTime = mediaCompressTime;
        this.decompressionTime = decompressionTime;
        this.maxDecompressTime = maxDecompressTime;
        this.minDecompressTime = minDecompressTime;
        this.mediaDecompressTime = mediaDecompressTime;
    }

    public ResultStructure(String filename, String compressorName, double compressorRatio, List<Double> compressionTime, List<Double> decompressionTime) {
        this.filename = filename;
        this.compressorName = compressorName;
        this.compressorRatio = compressorRatio;
        this.compressionTime = avgValue(compressionTime);
        this.maxCompressTime = maxValue(compressionTime);
        this.minCompressTime = minValue(compressionTime);
        this.mediaCompressTime = medianValue(compressionTime);
        this.decompressionTime = avgValue(decompressionTime);
        this.maxDecompressTime = maxValue(decompressionTime);
        this.minDecompressTime = minValue(decompressionTime);
        this.mediaDecompressTime = medianValue(decompressionTime);
    }

    public double getMaxCompressTime() {
        return maxCompressTime;
    }

    public void setMaxCompressTime(double maxCompressTime) {
        this.maxCompressTime = maxCompressTime;
    }

    public double getMinCompressTime() {
        return minCompressTime;
    }

    public void setMinCompressTime(double minCompressTime) {
        this.minCompressTime = minCompressTime;
    }

    public double getMediaCompressTime() {
        return mediaCompressTime;
    }

    public void setMediaCompressTime(double mediaCompressTime) {
        this.mediaCompressTime = mediaCompressTime;
    }

    public double getMaxDecompressTime() {
        return maxDecompressTime;
    }

    public void setMaxDecompressTime(double maxDecompressTime) {
        this.maxDecompressTime = maxDecompressTime;
    }

    public double getMinDecompressTime() {
        return minDecompressTime;
    }

    public void setMinDecompressTime(double minDecompressTime) {
        this.minDecompressTime = minDecompressTime;
    }

    public double getMediaDecompressTime() {
        return mediaDecompressTime;
    }

    public void setMediaDecompressTime(double mediaDecompressTime) {
        this.mediaDecompressTime = mediaDecompressTime;
    }

    public String getFilename() {
        return filename;
    }

    public void setFilename(String filename) {
        this.filename = filename;
    }

    public String getCompressorName() {
        return compressorName;
    }

    public void setCompressorName(String compressorName) {
        this.compressorName = compressorName;
    }

    public double getCompressorRatio() {
        return compressorRatio;
    }

    public void setCompressorRatio(double compressorRatio) {
        this.compressorRatio = compressorRatio;
    }

    public double getCompressionTime() {
        return compressionTime;
    }

    public void setCompressionTime(double compressionTime) {
        this.compressionTime = compressionTime;
    }

    public double getDecompressionTime() {
        return decompressionTime;
    }

    public void setDecompressionTime(double decompressionTime) {
        this.decompressionTime = decompressionTime;
    }

    public static String getHead(){
        return "dataset," +
                        "compression_tpc," +
                        "decompression_tpc" +
                        '\n';
    }

    public static String getCRHead(){
        return "dataset," +
                "size" +
                '\n';
    }

    @Override
    public String toString() {
        return mapDatasetName(filename.replace(".bin", "")) + ',' +
                compressionTime + ',' +
                decompressionTime +
                '\n';
    }

    public String toCRString() {
        return mapDatasetNameCR(filename.replace(".bin", "")) + ',' +
                (compressorRatio * 64) +
                '\n';
    }

    public double medianValue(List<Double> ld) {
        int num = ld.size();
        ld.sort(Comparator.naturalOrder());
        if (num % 2 == 1) {
            return ld.get(num / 2);
        } else {
            return (ld.get(num / 2) + ld.get(num / 2 - 1)) / 2;
        }
    }

    public double avgValue(List<Double> ld) {
        int num = ld.size();
        double al = 0;
        for (Double aDouble : ld) {
            al += aDouble;
        }
        return al / num;
    }

    public double maxValue(List<Double> ld) {
        int num = ld.size();
        double max = 0;
        for (Double aDouble : ld) {
            if(aDouble>max){
                max=aDouble;
            }
        }
        return max;
    }

    public double minValue(List<Double> ld) {
        double min = DoubleConsts.MAX_VALUE;
        for (Double aDouble : ld) {
            if(aDouble<min){
                min=aDouble;
            }
        }
        return min;
    }

    public double quarterLowValue(List<Double> ld) {
        int num = ld.size();
        ld.sort(Comparator.naturalOrder());
        return ld.get(num / 4);
    }

    public double quarterHighValue(List<Double> ld) {
        int num = ld.size();
        ld.sort(Comparator.naturalOrder());
        return ld.get(num * 3 / 4);
    }
}
