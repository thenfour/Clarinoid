using System;
using System.Collections.Generic;
using System.Drawing.Imaging;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;

namespace RhythmicAlignmentVisualizer
{
    public class DitherMatrix
    {
        private readonly int[,] _values;
        private readonly int[,] _valuesScaled;
        private Stopwatch sw = Stopwatch.StartNew();

        private int MatrixSize { get; }
        private int MaxThreshold { get; }

        /// <summary>
        /// The factor that accounts for (brightness / 256f) * (MaxThreshold+1)
        /// => brightness > threshold * Scale
        /// </summary>
        //public int Scale { get; }

        public DitherMatrix(int[,] values)
        {
            if (values == null)
                throw new ArgumentNullException(nameof(values));

            _values = values;

            // Determine matrix dimensions (assuming square matrix)
            MatrixSize = _values.GetLength(0);
            if (_values.GetLength(1) != MatrixSize)
                throw new ArgumentException("Dither matrix must be square.");

            // Compute MaxThreshold by finding the maximum value in the matrix
            MaxThreshold = FindMaxThreshold(_values);

            // Compute Scale factor
            var Scale = 256 / (MaxThreshold + 1);

            // Precompute the scaled thresholds
            _valuesScaled = new int[MatrixSize, MatrixSize];
            for (int y = 0; y < MatrixSize; y++)
            {
                for (int x = 0; x < MatrixSize; x++)
                {
                    _valuesScaled[y, x] = (_values[y, x] + 1) * Scale;
                }
            }
        }

        /// <summary>
        /// Finds the maximum threshold value in the provided matrix.
        /// </summary>
        /// <param name="matrix">2D array representing the dithering matrix.</param>
        /// <returns>Maximum threshold value.</returns>
        private int FindMaxThreshold(int[,] matrix)
        {
            int max = int.MinValue;
            for (int y = 0; y < matrix.GetLength(0); y++)
            {
                for (int x = 0; x < matrix.GetLength(1); x++)
                {
                    if (matrix[y, x] > max)
                        max = matrix[y, x];
                }
            }
            return max;
        }

        public int GetScaledThreshold(int x, int y)
        {
            // animation is not a good look. kinda ok in highly animated stuff but not worth it.
            int animOffset = 0;// (int)Math.Floor(sw.ElapsedMilliseconds / 50.0);
            //int animOffset =(int)Math.Floor(sw.ElapsedMilliseconds / 50.0);
            int matrixX = (animOffset + x) % MatrixSize;
            int matrixY = (animOffset + y) % MatrixSize;
            return _valuesScaled[matrixX, matrixY];
        }

        public bool GetDitheredColor(int shade, int x, int y)
        {
            var thresh = GetScaledThreshold(x, y) - 2;
            return shade > thresh;
        }

    }

    public class DoubleBufferedPictureBox : PictureBox
    {
        public DoubleBufferedPictureBox()
        {
            this.DoubleBuffered = true;
            this.SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
        }
    }


    public class FastDisplay : IDisplay
    {
        private Bitmap bitmap;
        private BitmapData bitmapData;
        private byte[] pixelBuffer;
        private int bytesPerPixel;
        private int stride;
        private int width;
        private int height;

        public FastDisplay(int width, int height)
        {
            this.width = width;
            this.height = height;

            bitmap = new Bitmap(width, height, PixelFormat.Format24bppRgb);
        }

        public Bitmap GetBitmap()
        {
            return bitmap;
        }

        public void BeginFrame()
        {
            bitmapData = bitmap.LockBits(new Rectangle(0, 0, width, height),
                                         ImageLockMode.ReadWrite,
                                         bitmap.PixelFormat);
            bytesPerPixel = Image.GetPixelFormatSize(bitmap.PixelFormat) / 8;
            stride = bitmapData.Stride;
            int totalBytes = stride * height;
            pixelBuffer = new byte[totalBytes];
            Marshal.Copy(bitmapData.Scan0, pixelBuffer, 0, totalBytes);
        }
        public void EndFrame()
        {
            Marshal.Copy(pixelBuffer, 0, bitmapData.Scan0, pixelBuffer.Length);
            bitmap.UnlockBits(bitmapData);
        }


        public void ClearDisplay()
        {
            Array.Clear(pixelBuffer, 0, pixelBuffer.Length);
        }

        public void SetPixel(int x, int y, bool on)
        {
            if (x < 0 || x >= bitmap.Width || y < 0 || y >= bitmap.Height)
                return;

            int index = y * stride + x * bytesPerPixel;
            if (on)
            {
                pixelBuffer[index] = 255;     // Blue
                pixelBuffer[index + 1] = 255; // Green
                pixelBuffer[index + 2] = 255; // Red
            }
            else
            {
                pixelBuffer[index] = 0;
                pixelBuffer[index + 1] = 0;
                pixelBuffer[index + 2] = 0;
            }
        }

        public void DrawHLine(int y, int xStart, int xEnd)
        {
            if (y < 0 || y >= bitmap.Height)
                return;

            for (int x = xStart; x <= xEnd && x < bitmap.Width; x++)
            {
                SetPixel(x, y, true);
            }
        }

        public void DrawVLine(int x, int yStart, int yEnd)
        {
            if (x < 0 || x >= bitmap.Width)
                return;

            for (int y = yStart; y <= yEnd && y < bitmap.Height; y++)
            {
                SetPixel(x, y, true);
            }
        }

        public void FillRect(int x, int y, int width, int height)
        {
            for (int i = x; i < x + width && i < bitmap.Width; i++)
            {
                for (int j = y; j < y + height && j < bitmap.Height; j++)
                {
                    SetPixel(i, j, true);
                }
            }
        }

        // outlined circle
        public void DrawCircle(float cx, float cy, float r)
        {
            int steps = 360;
            for (int deg = 0; deg < steps; deg++)
            {
                float rad = deg * (float)Math.PI / 180f;
                float x = cx + r * (float)Math.Cos(rad);
                float y = cy + r * (float)Math.Sin(rad);
                int pixelX = (int)x;
                int pixelY = (int)y;
                SetPixel(pixelX, pixelY, true);
            }
        }

        private static readonly int[,] Bayer2x2 =
        {
    { 0, 2 },
    { 3, 1 }
};

        public static readonly DitherMatrix Bayer2x2Matrix = new DitherMatrix(Bayer2x2);


        public static readonly DitherMatrix Bayer4x4Matrix = new DitherMatrix(new[,]{
    {  0, 8, 2, 10 },
    { 12,  4, 14,  6 },
    { 3, 11,  1,  9 },
    { 15,  7, 13,  5 }
});

        public static readonly DitherMatrix Bayer8x8Matrix = new DitherMatrix(new[,]{
    {  0, 48, 12, 60, 3, 51, 15, 63 },
    { 32, 16, 44, 28, 35, 19, 47, 31 },
    { 8, 56, 4, 52, 11, 59, 7, 55 },
    { 40, 24, 36, 20, 43, 27, 39, 23 },
    { 2, 50, 14, 62, 1, 49, 13, 61 },
    { 34, 18, 46, 30, 33, 17, 45, 29 },
    { 10, 58, 6, 54, 9, 57, 5, 53 },
    { 42, 26, 38, 22, 41, 25, 37, 21 }
});








        public bool IsInBounds(int x, int y)
        {
            if (x < 0 || y < 0) return false;
            if (x > width || y > height) return false;
            return true;
        }

        /// <summary>
        /// Plots a single pixel with given coverage (0..255) by dithering
        /// on a 1-bit display, using the specified DitherMatrix.
        /// </summary>
        public void SetPixelShaded(
            int x, int y,
            int coverage,      // 0..255
            DitherMatrix matrix)
        {
            // If out of bounds of your display, skip
            // (you can also do this check externally)
            if (!IsInBounds(x, y))
            {
                return;
            }

            // 1) Possibly handle negative coverage or clamp coverage
            if (coverage < 0) coverage = 0;
            if (coverage > 255) coverage = 255;

            SetPixel(x, y, matrix.GetDitheredColor(coverage, x, y));
        }

        public void SetPixelShaded(int x, int y, float brightness01, DitherMatrix matrix)
        {
            SetPixelShaded(x, y, (int)Math.Round(brightness01 * 255), matrix);
        }

        /// <summary>
        /// Draws a horizontal line of 'length' pixels starting at (x, y),
        /// dithering each pixel according to 'brightness' and the DitherMatrix.
        /// </summary>
        public void DrawHLineDithered(
            DitherMatrix matrix,
            int x, int y,
            int length,
            int brightness)
        {
            if (length <= 0) return;

            // Simple bounds check if needed
            // if your display is guaranteed to handle out-of-bounds, you can skip
            // or clamp length accordingly.

            for (int i = 0; i < length; i++)
            {
                int currentX = x + i;
                if (!IsInBounds(currentX, y)) continue;
                // your same set-pixel approach:
                //   setPixel( currentX, y, matrix.GetDitheredColor(brightness, currentX, y) );
                SetPixel(currentX, y, matrix.GetDitheredColor(brightness, currentX, y));
            }
        }

        /// <summary>
        /// Fills a rectangle at (x, y) of size width x height
        /// with simulated brightness using the specified DitherMatrix
        /// (purely in fixed-point integer arithmetic).
        /// </summary>
        /// <param name="matrix">The dither matrix to use.</param>
        /// <param name="x">Left X coordinate of the rectangle.</param>
        /// <param name="y">Top Y coordinate of the rectangle.</param>
        /// <param name="width">Rectangle width.</param>
        /// <param name="height">Rectangle height.</param>
        /// <param name="brightness">Brightness level (0-255).</param>
        public void FillRectWithBrightness(
            DitherMatrix matrix,
            int x, int y,
            int width, int height,
            int brightness)
        {
            // Clamp brightness
            if (brightness < 0) brightness = 0;
            if (brightness > 255) brightness = 255;

            for (int row = 0; row < height; row++)
            {
                int currentY = y + row;
                for (int col = 0; col < width; col++)
                {
                    int currentX = x + col;
                    SetPixel(currentX, currentY, matrix.GetDitheredColor(brightness, currentX, currentY));
                }
            }
        }

        /// <summary>
        /// Fills a circle of radius r centered at (x0,y0) with the given brightness,
        /// using a DitherMatrix for 1-bit dithering. No floating-point is used.
        /// </summary>
        public void FillCircleWithBrightness(
            DitherMatrix matrix,
            int x0,   // center X
            int y0,   // center Y
            int r,    // radius
            int brightness)
        {
            // Clamp brightness to [0..255]
            if (brightness < 0) brightness = 0;
            if (brightness > 255) brightness = 255;
            if (r < 0) return;  // no valid radius

            // Midpoint circle algorithm setup
            int x = 0;
            int y = r;

            // f is our "decision" variable, starts at 1 - r
            int f = 1 - r;
            // ddF_x, ddF_y track derivative changes
            int ddF_x = 1;
            int ddF_y = -2 * r;

            // We'll fill from the center's horizontal line out
            // so first fill the horizontal line across the circle's diameter
            // y=0 => from (x0-r) to (x0+r)
            DrawHLineDithered(matrix, x0 - r, y0, (2 * r + 1), brightness);

            // Also fill the symmetrical horizontal lines above & below
            // for each step of x,y as we move around the circle edges
            while (x < y)
            {
                // If f >= 0, move y inward
                if (f >= 0)
                {
                    y--;
                    ddF_y += 2;
                    f += ddF_y;
                }
                // Always move x outward
                x++;
                ddF_x += 2;
                f += ddF_x;

                // Now we have a circle boundary at (x,y). We fill horizontal lines:
                // "Top"  side at y0 + y
                // "Bottom" side at y0 - y
                // each goes from (x0 - x) to (x0 + x)

                DrawHLineDithered(matrix, x0 - x, y0 + y, (2 * x + 1), brightness);
                if (y != 0)  // if y=0, top & bottom would be same line
                {
                    DrawHLineDithered(matrix, x0 - x, y0 - y, (2 * x + 1), brightness);
                }

                // For x != y, fill those "side" lines near (y,x) due to circle symmetry:
                //   left side at x0 - y .. x0 + y, top y= y0 + x
                //   left side at x0 - y .. x0 + y, bottom y= y0 - x
                if (x != y)
                {
                    DrawHLineDithered(matrix, x0 - y, y0 + x, (2 * y + 1), brightness);
                    if (x != 0)  // if x=0, same line repeated
                    {
                        DrawHLineDithered(matrix, x0 - y, y0 - x, (2 * y + 1), brightness);
                    }
                }
            }
        }

        // Example helper to find the two points in 'points' that are farthest apart.
        private void FindFarthestPair(List<PointF> points, out PointF bestA, out PointF bestB)
        {
            bestA = points[0];
            bestB = points[0];
            float maxDistSq = 0f;

            // Simple O(n^2) check (fine for up to 4 points):
            for (int i = 0; i < points.Count; i++)
            {
                for (int j = i + 1; j < points.Count; j++)
                {
                    float dx = points[i].X - points[j].X;
                    float dy = points[i].Y - points[j].Y;
                    float distSq = dx * dx + dy * dy;
                    if (distSq > maxDistSq)
                    {
                        maxDistSq = distSq;
                        bestA = points[i];
                        bestB = points[j];
                    }
                }
            }
        }


        /// <summary>
        /// Draws a line from (x0, y0) to (x1, y1) with simulated brightness using
        /// the given DitherMatrix for Bayer dithering (purely in fixed-point).
        /// </summary>
        /// <param name="matrix">The dither matrix to use (e.g. 2x2, 4x4).</param>
        /// <param name="x0">Starting X coordinate.</param>
        /// <param name="y0">Starting Y coordinate.</param>
        /// <param name="x1">Ending X coordinate.</param>
        /// <param name="y1">Ending Y coordinate.</param>
        /// <param name="brightness">Brightness level (0-255).</param>
        public void DrawLineWithBrightness(
            DitherMatrix matrix,
            int x0, int y0,
            int x1, int y1,
            int brightness)
        {
            // Clamp brightness
            if (brightness < 0) brightness = 0;
            if (brightness > 255) brightness = 255;

            int dx = Math.Abs(x1 - x0);
            int dy = Math.Abs(y1 - y0);

            int sx = (x0 < x1) ? 1 : -1;
            int sy = (y0 < y1) ? 1 : -1;

            int err = dx - dy;
            int x = x0;
            int y = y0;

            while (true)
            {
                SetPixel(x, y, matrix.GetDitheredColor(brightness, x, y));

                if (x == x1 && y == y1) break;

                int e2 = 2 * err;
                if (e2 > -dy) { err -= dy; x += sx; }
                if (e2 < dx) { err += dx; y += sy; }
            }
        }

        // Specialized function:
        //   draws the infinite line through (x0,y0)-(x1,y1),
        //   clipped to [clipLeft..clipRight] x [clipTop..clipBottom].
        public void DrawInfiniteLineClipped(
            int x0, int y0,
            int x1, int y1,
            int clipLeft, int clipTop,
            int clipRight, int clipBottom,
            DitherMatrix matrix,
            int brightness)
        {
            // 1) Handle trivial cases: vertical / horizontal
            if (x0 == x1)
            {
                // Vertical line x=x0
                if (x0 < clipLeft || x0 > clipRight) return; // outside
                int yStart = clipTop;
                int yEnd = clipBottom;
                // Just draw from (x0, yStart) to (x0, yEnd)
                DrawLineWithBrightness(
                    matrix,
                    x0, yStart,
                    x0, yEnd,
                    brightness);
                return;
            }

            if (y0 == y1)
            {
                // Horizontal line y=y0
                if (y0 < clipTop || y0 > clipBottom) return; // outside
                int xStart = clipLeft;
                int xEnd = clipRight;
                // Draw from (xStart, y0) to (xEnd, y0)
                DrawLineWithBrightness(
                    matrix,
                    xStart, y0,
                    xEnd, y0,
                    brightness);
                return;
            }

            // 2) General case
            float slope = (float)(y1 - y0) / (float)(x1 - x0);

            // We'll track up to 4 intersection candidates:
            List<PointF> candidates = new List<PointF>(4);

            // Intersection at x=clipLeft => y = y0 + slope*(clipLeft - x0)
            float yLeft = y0 + slope * (clipLeft - x0);
            if (yLeft >= clipTop && yLeft <= clipBottom)
            {
                candidates.Add(new PointF(clipLeft, yLeft));
            }

            // Intersection at x=clipRight => y = y0 + slope*(clipRight - x0)
            float yRight = y0 + slope * (clipRight - x0);
            if (yRight >= clipTop && yRight <= clipBottom)
            {
                candidates.Add(new PointF(clipRight, yRight));
            }

            // Intersection at y=clipTop => x = x0 + (clipTop - y0)/slope
            float xTop = x0 + (clipTop - y0) / slope;
            if (xTop >= clipLeft && xTop <= clipRight)
            {
                candidates.Add(new PointF(xTop, clipTop));
            }

            // Intersection at y=clipBottom => x = x0 + (clipBottom - y0)/slope
            float xBottom = x0 + (clipBottom - y0) / slope;
            if (xBottom >= clipLeft && xBottom <= clipRight)
            {
                candidates.Add(new PointF(xBottom, clipBottom));
            }

            // Remove duplicates or near-duplicates if they happen (optional).
            // If there's no valid intersection => no draw
            if (candidates.Count < 2)
                return;

            // We only need two extremes. Possibly we have more if the line hits exactly a corner, etc.
            // Let's pick the two that are farthest from each other:
            //  (One way: measure the bounding box in that set.)
            float minX = float.MaxValue;
            float maxX = float.MinValue;
            float minY = float.MaxValue;
            float maxY = float.MinValue;

            foreach (var c in candidates)
            {
                if (c.X < minX) minX = c.X;
                if (c.X > maxX) maxX = c.X;
                if (c.Y < minY) minY = c.Y;
                if (c.Y > maxY) maxY = c.Y;
            }

            // Because it's a straight line, the "two extremes" in your intersection list
            // will either share the same min or max in both X and Y, or you can pick any
            // pair that yields the maximum distance. For simplicity, let's just pick
            // the minX-based intersection and the maxX-based intersection if the slope
            // is not near-infinite. That covers the typical case.

            // But to be robust, let's do a small function that picks the two
            // intersection points in 'candidates' that are farthest apart:
            PointF pA, pB;
            FindFarthestPair(candidates, out pA, out pB);

            // Convert float coords -> int, possibly rounding
            int ixA = (int)Math.Round(pA.X);
            int iyA = (int)Math.Round(pA.Y);
            int ixB = (int)Math.Round(pB.X);
            int iyB = (int)Math.Round(pB.Y);

            // Finally, draw using your dithering line routine (Bresenham-based):
            DrawLineWithBrightness(
                matrix,
                ixA, iyA,
                ixB, iyB,
                brightness);
        }

        ///// <summary>
        ///// Draw an anti-aliased line between (x0, y0) and (x1, y1),
        ///// using Xiaolin Wu's method in fixed-point, with dithering 
        ///// to a 1-bit display. 
        ///// 
        ///// brightness = [0..255], for scaling coverage if you want a dim line.
        ///// If you want full brightness, pass 255.
        ///// </summary>
        //public void DrawLineWuFixedPoint(
        //    int x0, int y0,
        //    int x1, int y1,
        //    int brightness,
        //    DitherMatrix matrix)
        //{
        //    // We can handle negative brightness, etc. 
        //    if (brightness < 0) brightness = 0;
        //    if (brightness > 255) brightness = 255;

        //    // We'll handle direction so we always draw left -> right 
        //    // if the slope is less than 1 in magnitude, else we swap roles of x & y.
        //    int dx = x1 - x0;
        //    int dy = y1 - y0;

        //    bool steep = (Math.Abs(dy) > Math.Abs(dx));
        //    if (steep)
        //    {
        //        // swap x,y for both points
        //        // We'll interpret the line as "vertical-ish"
        //        // so we iterate in y. 
        //        // A standard trick is to swap them so we can reuse the 'low slope' code.
        //        Swap(ref x0, ref y0);
        //        Swap(ref x1, ref y1);
        //        Swap(ref dx, ref dy);
        //    }

        //    if (x0 > x1)
        //    {
        //        // Ensure we go left -> right
        //        Swap(ref x0, ref x1);
        //        Swap(ref y0, ref y1);
        //        dx = x1 - x0;
        //        dy = y1 - y0;
        //    }

        //    // Now we do the typical Wu's line for dx >= 0 
        //    // If slope is negative, dy might be negative.
        //    // We'll use a 16.16 fixed for our "y" 
        //    // slopeFixed = (dy << 16) / dx
        //    // but be careful about dx=0 edge case if line is vertical after swapping.

        //    if (dx == 0)
        //    {
        //        // vertical line after all
        //        // Just do a fallback: all y from y0..y1, full coverage
        //        // This won't be "antialiased" for slope, 
        //        // but you can fill in a standard approach if you'd like.
        //        // Or handle separately. 
        //        int startY = Math.Min(y0, y1);
        //        int endY = Math.Max(y0, y1);
        //        for (int y = startY; y <= endY; y++)
        //        {
        //            if (!steep) SetPixelShaded(x0, y, brightness, matrix);
        //            else SetPixelShaded(y, x0, brightness, matrix);
        //        }
        //        return;
        //    }

        //    // slope in 16.16
        //    // (could overflow if dx,dy ~ 30k each; typically won't for small screens. 
        //    //  if worried, use 64-bit intermediate)
        //    int slopeFixed = (dy << 16) / dx;  // may be negative if line slopes down

        //    // currentY in 16.16
        //    int currentY = y0 << 16;

        //    // We step x from x0..x1
        //    for (int x = x0; x <= x1; x++)
        //    {
        //        // integer part of Y
        //        int yInt = currentY >> 16;
        //        // fractional part 0..65535
        //        int frac = currentY & 0xFFFF;

        //        // coverage for top pixel: (1 - fraction)
        //        // coverage for bottom pixel: fraction
        //        // We'll do them in [0..255].
        //        int covBot = frac >> 8;            // range 0..255
        //        int covTop = 255 - covBot;         // also 0..255

        //        // If slope >=0, the "bottom" pixel is yInt+1. If slope<0, it's yInt-1.
        //        // Actually, Wu's typical formula: 
        //        //   "main" pixel coverage = 1 - frac => yInt
        //        //   "adjacent" pixel coverage = frac => yInt+1
        //        // That is for a line going from top-left to bottom-right. 
        //        // If slope <0, the line is going downward, 
        //        // but the adjacency is yInt - 1. Let's handle that:

        //        int adjY;
        //        int mainCov, adjCov;

        //        if (slopeFixed >= 0)
        //        {
        //            // top pixel is yInt, coverage = covTop
        //            // bottom pixel is yInt+1, coverage = covBot
        //            adjY = yInt + 1;
        //            mainCov = covTop;
        //            adjCov = covBot;
        //        }
        //        else
        //        {
        //            // the line slopes downward, so the "second" pixel is above yInt
        //            // top pixel is yInt, coverage = covBot
        //            // adjacent pixel is yInt-1, coverage = covTop
        //            adjY = yInt - 1;
        //            mainCov = covBot;
        //            adjCov = covTop;
        //        }

        //        // Also apply global brightness scale:
        //        mainCov = (mainCov * brightness) >> 8;  // 0..255
        //        adjCov = (adjCov * brightness) >> 8;  // 0..255

        //        // Now plot them. If 'steep' is true, we swapped x,y earlier:
        //        if (!steep)
        //        {
        //            // normal
        //            SetPixelShaded(x, yInt, mainCov, matrix);
        //            SetPixelShaded(x, adjY, adjCov, matrix);
        //        }
        //        else
        //        {
        //            // swapped
        //            SetPixelShaded(yInt, x, mainCov, matrix);
        //            SetPixelShaded(adjY, x, adjCov, matrix);
        //        }

        //        // Move to next x
        //        currentY += slopeFixed;
        //    }
        //}

        ///// <summary>
        ///// Helper to swap two ints by reference
        ///// </summary>
        //private void Swap(ref int a, ref int b)
        //{
        //    int tmp = a;
        //    a = b;
        //    b = tmp;
        //}
    }
}