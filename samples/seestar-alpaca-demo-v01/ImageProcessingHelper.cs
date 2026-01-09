using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace alpaca_alpaca_demo
{
    internal static class ImageProcessingHelper
    {
        public static Bitmap ConvertImageArrayToBitmap(Array imageArray)
        {
            if (imageArray == null)
            {
                throw new ArgumentNullException(nameof(imageArray));
            }

            if (imageArray.Rank != 2)
            {
                throw new ArgumentException("仅支持二维图像数组", nameof(imageArray));
            }

            int width = imageArray.GetLength(0);
            int height = imageArray.GetLength(1);
            if (width == 0 || height == 0)
            {
                throw new ArgumentException("图像数据尺寸无效", nameof(imageArray));
            }

            ushort[] flatArray = FlipAndConvert2d(imageArray, width, height);
            int length = flatArray.Length;

            ushort minValue = flatArray[0];
            ushort maxValue = flatArray[0];
            for (int i = 1; i < length; i++)
            {
                ushort value = flatArray[i];
                if (value < minValue)
                {
                    minValue = value;
                }
                else if (value > maxValue)
                {
                    maxValue = value;
                }
            }

            double scale = maxValue > minValue ? 255.0 / (maxValue - minValue) : 0.0;
            Bitmap bitmap = new Bitmap(width, height, PixelFormat.Format24bppRgb);
            BitmapData data = bitmap.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);

            ushort GetPixel(int px, int py)
            {
                return flatArray[(py * width) + px];
            }

            double Average(params (int X, int Y)[] positions)
            {
                double sum = 0;
                int count = 0;

                foreach (var (px, py) in positions)
                {
                    if (px >= 0 && px < width && py >= 0 && py < height)
                    {
                        sum += GetPixel(px, py);
                        count++;
                    }
                }

                if (count == 0)
                {
                    return 0;
                }

                return sum / count;
            }

            double Clamp(double value, double min, double max)
            {
                if (value < min)
                {
                    return min;
                }

                if (value > max)
                {
                    return max;
                }

                return value;
            }

            byte ScaleToByte(double value)
            {
                if (maxValue > minValue)
                {
                    value = Clamp(value, minValue, maxValue);
                    int scaled = (int)Math.Round((value - minValue) * scale);
                    if (scaled < 0)
                    {
                        scaled = 0;
                    }
                    else if (scaled > 255)
                    {
                        scaled = 255;
                    }

                    return (byte)scaled;
                }

                int direct = (int)Math.Round(value);
                if (direct < 0)
                {
                    direct = 0;
                }
                else if (direct > 255)
                {
                    direct = 255;
                }

                return (byte)direct;
            }

            byte[] buffer = new byte[data.Stride * height];
            for (int y = 0; y < height; y++)
            {
                int bufferOffset = y * data.Stride;

                for (int x = 0; x < width; x++)
                {
                    ushort current = GetPixel(x, y);
                    bool rowEven = (y & 1) == 0;
                    bool colEven = (x & 1) == 0;

                    double red;
                    double green;
                    double blue;

                    if (rowEven)
                    {
                        if (colEven)
                        {
                            // Green pixel on a red row
                            green = current;
                            red = Average((x - 1, y), (x + 1, y));
                            blue = Average((x, y - 1), (x, y + 1));
                        }
                        else
                        {
                            // Red pixel
                            red = current;
                            green = Average((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1));
                            blue = Average((x - 1, y - 1), (x + 1, y - 1), (x - 1, y + 1), (x + 1, y + 1));
                        }
                    }
                    else
                    {
                        if (colEven)
                        {
                            // Blue pixel
                            blue = current;
                            green = Average((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1));
                            red = Average((x - 1, y - 1), (x + 1, y - 1), (x - 1, y + 1), (x + 1, y + 1));
                        }
                        else
                        {
                            // Green pixel on a blue row
                            green = current;
                            red = Average((x, y - 1), (x, y + 1));
                            blue = Average((x - 1, y), (x + 1, y));
                        }
                    }

                    int pixelOffset = bufferOffset + (x * 3);
                    buffer[pixelOffset] = ScaleToByte(blue);
                    buffer[pixelOffset + 1] = ScaleToByte(green);
                    buffer[pixelOffset + 2] = ScaleToByte(red);
                }
            }

            Marshal.Copy(buffer, 0, data.Scan0, buffer.Length);
            bitmap.UnlockBits(data);

            return bitmap;
        }

        private static ushort[] FlipAndConvert2d(Array input, int width, int height)
        {
            if (input is byte[,] byteArray)
            {
                return ProcessAsByte(byteArray, width, height);
            }
            else if (input is short[,] shortArray)
            {
                return ProcessAsShort(shortArray, width, height);
            }
            else if (input is ushort[,] ushortArray)
            {
                return ProcessAsUShort(ushortArray, width, height);
            }
            else if (input is uint[,] uintArray)
            {
                return ProcessAsUInt(uintArray, width, height);
            }
            else if (input is int[,] intArray)
            {
                return ProcessAsInt(intArray, width, height);
            }
            else
            {
                throw new NotSupportedException("Unsupported data type");
            }
        }

        private static ushort[] ProcessAsByte(byte[,] arr, int width, int height)
        {
            ushort[] flatArray = new ushort[width * height];
            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    flatArray[(y * width) + x] = arr[x, y];
                }
            }

            return flatArray;
        }

        private static ushort[] ProcessAsShort(short[,] arr, int width, int height)
        {
            ushort[] flatArray = new ushort[width * height];
            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    flatArray[(y * width) + x] = (ushort)arr[x, y];
                }
            }

            return flatArray;
        }

        private static ushort[] ProcessAsUShort(ushort[,] arr, int width, int height)
        {
            ushort[] flatArray = new ushort[width * height];
            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    flatArray[(y * width) + x] = arr[x, y];
                }
            }

            return flatArray;
        }

        private static ushort[] ProcessAsUInt(uint[,] arr, int width, int height)
        {
            ushort[] flatArray = new ushort[width * height];
            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    flatArray[(y * width) + x] = (ushort)arr[x, y];
                }
            }

            return flatArray;
        }

        private static ushort[] ProcessAsInt(int[,] arr, int width, int height)
        {
            ushort[] flatArray = new ushort[width * height];
            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    flatArray[(y * width) + x] = (ushort)arr[x, y];
                }
            }

            return flatArray;
        }
    }
}
