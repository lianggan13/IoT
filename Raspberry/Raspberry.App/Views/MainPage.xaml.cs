using CommunityToolkit.Maui.Converters;
using System.Diagnostics;

namespace Raspberry.App.Views
{
    public partial class MainPage : ContentPage
    {
        int count = 0;

        public MainPage()
        {
            InitializeComponent();
        }

        private void Frame_Loaded(object sender, EventArgs e)
        {
            //img.Source = ImageSource.FromFile("a3.jpeg");
            //img.Source = ImageSource.FromResource("Raspberry.App.Resources.Images.a3.jpeg");
            //img.Source = ImageSource.FromResource("Raspberry.App.Resources.Images.a3.jpeg", typeof(MainPage).GetTypeInfo().Assembly);
        }

        private void OnCounterClicked(object sender, EventArgs e)
        {
            count++;

            //if (count == 1)
            //    CounterBtn.Text = $"Clicked {count} time";
            //else
            //    CounterBtn.Text = $"Clicked {count} times";

            //SemanticScreenReader.Announce(CounterBtn.Text);
        }

        private void btnW_Pressed(object sender, EventArgs e)
        {

            Button btn = sender as Button;
            Debug.WriteLine($"{btn.Text} pressed...");
        }

        private void btnW_Released(object sender, EventArgs e)
        {

            Button btn = sender as Button;
            Debug.WriteLine($"{btn.Text} released...");
        }

        private void OnEntryTextChanged(object sender, TextChangedEventArgs e)
        {

        }

        private void OnEntryCompleted(object sender, EventArgs e)
        {
            //Keys
        }

        private void Button_Clicked(object sender, EventArgs e)
        {
            TakePhoto();
        }

        public async void TakePhoto()
        {
            if (MediaPicker.Default.IsCaptureSupported)
            {
                FileResult photo = await MediaPicker.Default.CapturePhotoAsync();

                if (photo != null)
                {
                    // save the file into local storage
                    string localFilePath = Path.Combine(FileSystem.CacheDirectory, photo.FileName);

                    using Stream sourceStream = await photo.OpenReadAsync();
                    using FileStream localFileStream = File.OpenWrite(localFilePath);

                    await sourceStream.CopyToAsync(localFileStream);
                }
            }
        }
        private void Button2_Clicked(object sender, EventArgs e)
        {
            Task.Run(async () =>
            {
                while (true)
                {

                    Application.Current.Dispatcher.Dispatch(async () =>
                    {
                        imgCamera.Source = await TakeScreenshotAsync();
                    });

                    await Task.Delay(300);
                }
            });

        }

        ByteArrayToImageSourceConverter bytes2Image = new ByteArrayToImageSourceConverter();

        public async Task<ImageSource> TakeScreenshotAsync()
        {
            if (Screenshot.Default.IsCaptureSupported)
            {
                IScreenshotResult screen = await Screenshot.Default.CaptureAsync();

                Stream stream = await screen.OpenReadAsync();

                var bytes = bytes2Image.ConvertBackTo(ImageSource.FromStream(() => stream));

                return ImageSource.FromStream(() => new MemoryStream(bytes));
            }

            return null;
        }


    }
}