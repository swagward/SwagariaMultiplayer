package com.swagaria.util;

import java.util.Random;

//minimal open simplex noise for terrain gen
public class OpenSimplexNoise {
    private static final double STRETCH_2D = -0.211324865405187;
    private static final double SQUISH_2D = 0.366025403784439;
    private static final double NORM_2D = 47.0;
    private final short[] perm;

    public OpenSimplexNoise(long seed) {
        perm = new short[256];
        short[] source = new short[256];
        for (short i = 0; i < 256; i++) source[i] = i;
        Random rand = new Random(seed);
        for (int i = 255; i >= 0; i--) {
            int r = rand.nextInt(i + 1);
            perm[i] = source[r];
            source[r] = source[i];
        }
    }

    private static final double[] gradients2D = {
            5, 2, 2, 5,
            -5, 2, -2, 5,
            5, -2, 2, -5,
            -5, -2, -2, -5,
    };

    private static int fastFloor(double x) {
        int xi = (int)x;
        return x < xi ? xi - 1 : xi;
    }

    private static double extrapolate(short[] perm, int xsb, int ysb, double dx, double dy) {
        int index = perm[(perm[xsb & 0xFF] + ysb) & 0xFF] & 0x0E;
        return gradients2D[index] * dx + gradients2D[index + 1] * dy;
    }

    public double eval(double x, double y) {
        double stretchOffset = (x + y) * STRETCH_2D;
        double xs = x + stretchOffset;
        double ys = y + stretchOffset;

        int xsb = fastFloor(xs);
        int ysb = fastFloor(ys);

        double squishOffset = (xsb + ysb) * SQUISH_2D;
        double xb = xsb + squishOffset;
        double yb = ysb + squishOffset;

        double xins = xs - xsb;
        double yins = ys - ysb;
        double inSum = xins + yins;

        double dx0 = x - xb;
        double dy0 = y - yb;

        double value = 0.0;

        //contribution (1,0)
        double dx1 = dx0 - 1 - SQUISH_2D;
        double dy1 = dy0 - 0 - SQUISH_2D;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * extrapolate(perm, xsb + 1, ysb, dx1, dy1);
        }

        //contribution (0,1)
        double dx2 = dx0 - 0 - SQUISH_2D;
        double dy2 = dy0 - 1 - SQUISH_2D;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * extrapolate(perm, xsb, ysb + 1, dx2, dy2);
        }

        //contribution (0,0)
        double attn0 = 2 - dx0 * dx0 - dy0 * dy0;
        if (attn0 > 0) {
            attn0 *= attn0;
            value += attn0 * attn0 * extrapolate(perm, xsb, ysb, dx0, dy0);
        }

        return value / NORM_2D;
    }
}
