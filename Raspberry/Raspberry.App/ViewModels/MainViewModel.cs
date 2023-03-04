using CommunityToolkit.Maui.Converters;
using CommunityToolkit.Mvvm.ComponentModel;
using Raspberry.App.AttachedProperties;
using Raspberry.App.Services;
using Raspberry.App.Views;
using System.ComponentModel;
using System.Diagnostics;
using System.Windows.Input;

namespace Raspberry.App.ViewModels
{
    public class MainViewModel : ObservableObject
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private DateTime _dateTime;
        private Timer _timer;

        public DateTime DateTime
        {
            get => _dateTime;
            set
            {
                if (_dateTime != value)
                {
                    _dateTime = value;
                    OnPropertyChanged(); // reports this property
                }
            }
        }

        private ImageSource img;
        private SocketClient socketClient;

        public ImageSource Img
        {
            get { return img; }
            set
            {
                img = value;
                OnPropertyChanged();
            }
        }
        public ICommand MainPageLoadedCommand => new Command<object>(MainPageLoaded);
        public ICommand BtnPressedCommand => new Command<object>(Button_Pressed);
        public ICommand BtnReleasedCommand => new Command<object>(Button_Released);

        private byte[] dotNetBotImageByteArray;

        public byte[] DotNetBotImageByteArray
        {
            get { return dotNetBotImageByteArray; }
            set
            {
                dotNetBotImageByteArray = value;
                OnPropertyChanged();
            }
        }


        public MainViewModel()
        {
            this.DateTime = DateTime.Now;

            // Update the DateTime property every second.
            _timer = new Timer(new TimerCallback((s) => this.DateTime = DateTime.Now),
                               null, TimeSpan.Zero, TimeSpan.FromSeconds(1));
        }

        ~MainViewModel() =>
            _timer.Dispose();

        private void MainPageLoaded(object obj)
        {
            mainPage = (MainPage)obj;
            Task.Run(() =>
            {
                try
                {
                    socketClient = new SocketClient();
                    socketClient.Connect("192.168.0.9", 32769);
                    socketClient.Received += SocketClient_Received;
                }
                catch (System.Exception ex)
                {
                    Debug.Fail(ex.ToString());
                }
            });
        }

        ByteArrayToImageSourceConverter bytes2Image = new ByteArrayToImageSourceConverter();
        private MainPage mainPage;

        private void SocketClient_Received(object sender, byte[] data)
        {
            try
            {
                //HttpClient client = new HttpClient();
                //DotNetBotImageByteArray = await client.GetByteArrayAsync("https://user-images.githubusercontent.com/13558917/137551073-ac8958bf-83e3-4ae3-8623-4db6dce49d02.png").ConfigureAwait(false);
                //Img = bytes2Image.ConvertFrom(data);
                //DotNetBotImageByteArray = data;
                Application.Current.Dispatcher.Dispatch
                    (() =>
                    {
                        //  mainPage.imgCamera.Source = ImageSource.FromStream(() => new MemoryStream(data));

                    });
                //Bitmap bitmap = ImageHelper.Buffer2Bitmap(data);
                //System.Drawing.Image t_img = ImageHelper.AddTextToImg(bitmap, $"{DateTime.Now:HH:mm:ss}", 12.0f, bitmap.Width - 10, bitmap.Height - 10, 120, ImageFormat.Jpeg);

                //Application.Current?.Dispatcher.Invoke(() =>
                //{
                //    //Img = ImageHelper.ConvertByteArrayToBitmapImage(data);
                //    Img = ImageHelper.BitmapToBitmapImage(new Bitmap(t_img));
                //});

            }
            catch (Exception ex)
            {
                Debug.Fail(ex.ToString());
            }
        }

        private void Button_Pressed(object sender)
        {
            Button btn = sender as Button;
            Key key = ButtonKeyBoard.GetKey(btn);
            Debug.WriteLine($"{key} pressed...");
            socketClient.Send($"{key}");
        }

        private void Button_Released(object sender)
        {
            Button btn = sender as Button;
            Key key = ButtonKeyBoard.GetKey(btn);
            Debug.WriteLine($"{key} released...");
            socketClient.Send($"P");
        }


    }
}
