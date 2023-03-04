using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace Raspberry.Client.Utils
{
    public class ImageHelper
    {
        public static byte[] ConvertBitmapSourceToByteArray(BitmapEncoder encoder, ImageSource imageSource)
        {
            byte[] bytes = null;
            var bitmapSource = imageSource as BitmapSource;

            if (bitmapSource != null)
            {
                encoder.Frames.Add(BitmapFrame.Create(bitmapSource));

                using (var stream = new MemoryStream())
                {
                    encoder.Save(stream);
                    bytes = stream.ToArray();
                }
            }

            return bytes;
        }

        public static byte[] ConvertBitmapSourceToByteArray(BitmapSource image)
        {
            byte[] data;
            BitmapEncoder encoder = new JpegBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(image));
            using (MemoryStream ms = new MemoryStream())
            {
                encoder.Save(ms);
                data = ms.ToArray();
            }
            return data;
        }
        public static byte[] ConvertBitmapSourceToByteArray(ImageSource imageSource)
        {
            var image = imageSource as BitmapSource;
            byte[] data;
            BitmapEncoder encoder = new JpegBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(image));
            using (MemoryStream ms = new MemoryStream())
            {
                encoder.Save(ms);
                data = ms.ToArray();
            }
            return data;
        }
        public static byte[] ConvertBitmapSourceToByteArray(Uri uri)
        {
            var image = new BitmapImage(uri);
            byte[] data;
            BitmapEncoder encoder = new JpegBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(image));
            using (MemoryStream ms = new MemoryStream())
            {
                encoder.Save(ms);
                data = ms.ToArray();
            }
            return data;
        }
        public static byte[] ConvertBitmapSourceToByteArray(string filepath)
        {
            var image = new BitmapImage(new Uri(filepath));
            byte[] data;
            BitmapEncoder encoder = new JpegBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(image));
            using (MemoryStream ms = new MemoryStream())
            {
                encoder.Save(ms);
                data = ms.ToArray();
            }
            return data;
        }

        public static BitmapImage ConvertByteArrayToBitmapImage(byte[] bytes)
        {
            var stream = new MemoryStream(bytes);
            stream.Seek(0, SeekOrigin.Begin);
            var image = new BitmapImage();
            image.BeginInit();
            image.StreamSource = stream;
            image.EndInit();
            return image;
        }

        private ImageSource ToBitmapSourceA(Bitmap bitmap)
        {
            MemoryStream stream = new MemoryStream();
            bitmap.Save(stream, ImageFormat.Bmp);
            stream.Position = 0;
            BitmapImage bitmapImage = new BitmapImage();
            bitmapImage.BeginInit();
            bitmapImage.StreamSource = stream;
            bitmapImage.EndInit();
            return bitmapImage;
        }

        public static BitmapImage BitmapToBitmapImage(Bitmap bitmap)
        {
            Bitmap ImageOriginalBase = new Bitmap(bitmap);
            BitmapImage bitmapImage = new BitmapImage();
            using (MemoryStream ms = new MemoryStream())
            {
                ImageOriginalBase.Save(ms, ImageFormat.Png);
                bitmapImage.BeginInit();
                bitmapImage.StreamSource = ms;
                bitmapImage.CacheOption = BitmapCacheOption.OnLoad;
                bitmapImage.EndInit();
                bitmapImage.Freeze();
            }
            return bitmapImage;
        }

        public static Bitmap Buffer2Bitmap(byte[] buffer)
        {
            MemoryStream ms = new MemoryStream(buffer);
            Bitmap bmp = new Bitmap(ms);
            ms.Close();
            return bmp;
        }

        /// <summary>
        /// 添加文字水印
        /// </summary>
        /// <param name="image"></param>
        /// <param name="text"></param>
        /// <param name="fontSize">字体大小</param>
        /// <param name="rectX">水印开始X坐标（自动扣除文字宽度）</param>
        /// <param name="rectY">水印开始Y坐标（自动扣除文字高度</param>
        /// <param name="opacity">0-255 值越大透明度越低</param>
        /// <param name="externName">文件后缀名</param>
        /// <returns></returns>
        public static Image AddTextToImg(Image image, string text, float fontSize, float rectX, float rectY, int opacity, ImageFormat format)
        {
            Bitmap bitmap = new Bitmap(image, image.Width, image.Height);

            Graphics g = Graphics.FromImage(bitmap);

            //下面定义一个矩形区域            
            float rectWidth = text.Length * (fontSize + 10);
            float rectHeight = fontSize + 10;

            //声明矩形域

            RectangleF textArea = new RectangleF(rectX - rectWidth, rectY - rectHeight, rectWidth, rectHeight);

            Font font = new Font("微软雅黑", fontSize, FontStyle.Bold); //定义字体

            System.Drawing.Brush whiteBrush = new SolidBrush(System.Drawing.Color.FromArgb(opacity, 193, 143, 8)); //画文字用

            g.DrawString(text, font, whiteBrush, textArea);

            MemoryStream ms = new MemoryStream();

            //保存图片
            bitmap.Save(ms, format);

            g.Dispose();

            Image h_hovercImg = Image.FromStream(ms);

            ms.Close();
            ms.Dispose();
            bitmap.Dispose();

            return h_hovercImg;

        }
    }
}
