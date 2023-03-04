using Android.App;
using Android.Content.PM;

namespace Raspberry.App
{
    //https://learn.microsoft.com/en-us/xamarin/xamarin-forms/user-interface/layouts/device-orientation?tabs=windows
    [Activity(Theme = "@style/Maui.SplashTheme", MainLauncher = true,
            ConfigurationChanges = ConfigChanges.ScreenSize
                                    | ConfigChanges.Orientation
                                    | ConfigChanges.UiMode
                                    | ConfigChanges.ScreenLayout
                                    | ConfigChanges.SmallestScreenSize
                                    | ConfigChanges.Density)]
    //| ConfigChanges.Density, ScreenOrientation = ScreenOrientation.Landscape)]
    public class MainActivity : MauiAppCompatActivity
    {
    }

}