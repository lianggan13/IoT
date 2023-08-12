using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Media;

namespace Raspberry.Client.Utils
{
    public class UIControlHelper
    {
        public static List<T> GetChildObjects<T>(DependencyObject obj) where T : FrameworkElement
        {
            DependencyObject child = null;
            List<T> childList = new List<T>();

            for (int i = 0; i <= VisualTreeHelper.GetChildrenCount(obj) - 1; i++)
            {
                child = VisualTreeHelper.GetChild(obj, i);

                //if (child is T && ((T)child).GetType() == typename)
                if (child is T)
                {
                    childList.Add((T)child);
                }
                childList.AddRange(GetChildObjects<T>(child));
            }
            return childList;
        }

        public static List<T> GetLogicChildObjects<T>(DependencyObject obj) where T : FrameworkElement
        {
            List<T> childList = new List<T>();
            foreach (var child in LogicalTreeHelper.GetChildren(obj))
            {
                if (child is DependencyObject)
                {
                    if (child is T)
                    {
                        childList.Add((T)child);
                    }
                    childList.AddRange(GetLogicChildObjects<T>(child as DependencyObject));
                }
            }

            return childList;
        }


        public static List<T> FindVisualChild<T>(DependencyObject obj, Func<T, bool> where = null) where T : DependencyObject
        {
            List<T> childList = new List<T>();
            int n = VisualTreeHelper.GetChildrenCount(obj);
            for (int i = 0; i < n; i++)
            {
                DependencyObject child = VisualTreeHelper.GetChild(obj, i);
                if (child is DependencyObject dChild)
                {
                    if (child is T tChild && (where == null || where?.Invoke(tChild) == true))
                    {
                        childList.Add(tChild);
                    }
                    childList.AddRange(FindVisualChild(dChild, where));
                }
            }
            return childList;
        }

        public static T FindParent<T>(FrameworkElement element) where T : FrameworkElement
        {
            while (element != null)
            {
                if (element is T) return (T)element;

                element = VisualTreeHelper.GetParent(element) as FrameworkElement;
            }

            return default(T);
        }
    }
}
