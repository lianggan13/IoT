using Prism.Commands;
using Prism.Mvvm;
using Raspberry.Client.AttachedProperties;
using Raspberry.Client.Services;
using Raspberry.Client.Utils;
using Raspberry.Client.Views;
using System;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace Raspberry.Client.ViewModels
{
    public class MainViewModel : BindableBase
    {
        private string _title = "Raspberry Application";
        private SocketClient socketClient;

        public string Title
        {
            get { return _title; }
            set { SetProperty(ref _title, value); }
        }
        public DelegateCommand<object> PressKeyCommand => new DelegateCommand<object>(PressKey);


        private ImageSource img;

        public ImageSource Img
        {
            get { return img; }
            set { SetProperty(ref img, value); }
        }

        public MainViewModel()
        {

        }

        public void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Task.Run(() =>
            {
                try
                {
                    socketClient = new SocketClient();
                    socketClient.Connect("192.168.4.1", 32769);
                    ////socketClient.Connect("192.168.", 32769);
                    socketClient.Received += SocketClient_Received;
                }
                catch (System.Exception ex)
                {
                    Debug.Fail(ex.ToString());
                }
            });
        }

        private void SocketClient_Received(object arg1, byte[] data)
        {
            try
            {
                //System.Drawing.Bitmap bitmap = ImageHelper.Buffer2Bitmap(data);
                //System.Drawing.Image t_img = ImageHelper.AddTextToImg(bitmap, $"{DateTime.Now:HH:mm:ss}", 12.0f, bitmap.Width - 10, bitmap.Height - 10, 120, System.Drawing.Imaging.ImageFormat.Jpeg);

                Application.Current?.Dispatcher.Invoke(() =>
                {
                    Img = ImageHelper.ConvertByteArrayToBitmapImage(data);
                    //Img = ImageHelper.BitmapToBitmapImage(new System.Drawing.Bitmap(t_img));
                });

            }
            catch (Exception ex)
            {
                //Debug.Fail(ex.ToString());
            }
        }


        private void PressKey(object para)
        {
            Button btn = para as Button;
            if (!btn.IsFocused)
            {
                //btn.Focus();
                Keyboard.Focus(btn);
            }


            Key key = ButtonKeyBoard.GetKey(btn);
            Debug.WriteLine($"{nameof(PressKey)}:{key}");
            socketClient.Send($"{key}");
        }


        public void Btn_KeyUp(object sender, KeyEventArgs e)
        {
            if (sender is Button btn)
            {
                Key key = ButtonKeyBoard.GetKey(btn);
                if (key == e.Key)
                {
                    (Application.Current.MainWindow as MainWindow).imgHost.Focus();
                    Debug.WriteLine($"{nameof(Btn_KeyUp)}:{key}");
                    socketClient.Send($"P");
                }
            }
            e.Handled = true;
        }
    }
}
