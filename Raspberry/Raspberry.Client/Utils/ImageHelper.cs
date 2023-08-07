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

        /// <summary>
        /// 在WPF中，当使用Image控件加载并显示图片时，有时候图片文件会被锁定，导致无法完全释放资源。这是因为在显示图片时，WPF的BitmapImage会直接获取文件流并锁定它。为了避免这个问题，你可以使用以下方法来加载图片并完全释放资源
        /// 1.使用BitmapImage的CacheOption属性设置为BitmapCacheOption.OnLoad，这将会在加载图片时将其缓存在内存中，从而解除对文件的锁定。
        /// 2.使用MemoryStream来手动缓存图片，以释放文件资源。
        /// </summary>
        public static BitmapImage Bytes2BitmapImage(byte[] bytes)
        {
            //BitmapImage bmp = new BitmapImage();
            //using (MemoryStream stream = new MemoryStream(bytes))
            //{
            //    bmp.BeginInit();
            //    bmp.CacheOption = BitmapCacheOption.OnLoad; // 加载图片
            //    bmp.CreateOptions = BitmapCreateOptions.DelayCreation; // 延迟后使用
            //    bmp.StreamSource = stream;
            //    bmp.EndInit();
            //    bmp.Freeze();
            //}

            //bmp.BeginInit();
            //bmp.CacheOption = BitmapCacheOption.OnLoad; // 将会在加载图片时将其缓存在内存中，从而解除对文件的锁定
            ////bmp.CreateOptions = BitmapCreateOptions.DelayCreation; // 延迟后使用
            //bmp.StreamSource = new MemoryStream(bytes);
            //bmp.EndInit();
            //bmp.Freeze();

            // 创建一个MemoryStream，用于缓存图像数据
            var memoryStream = new MemoryStream(bytes);

            // 创建一个BitmapImage并设置CacheOption为OnLoad，以便立即解锁文件
            var bitmapImage = new BitmapImage();
            bitmapImage.BeginInit();
            bitmapImage.CacheOption = BitmapCacheOption.OnLoad;
            bitmapImage.StreamSource = memoryStream;
            bitmapImage.EndInit();

            // 必要时，手动清除MemoryStream资源
            memoryStream.Close();
            memoryStream.Dispose();

            return bitmapImage;
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
