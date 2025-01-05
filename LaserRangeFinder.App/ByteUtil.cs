using System.Runtime.InteropServices;

namespace TOFSense_F
{
    public class ByteUtil
    {
        public static bool[] ToBinary(byte b)
        {
            return Convert.ToString(b, 2).PadLeft(8, '0').Select(i => i == '1').ToArray();
        }

        public static byte ToByte(string binaryString)
        {
            if (binaryString == null
                || binaryString.Length <= 0
                || binaryString.Length > 8
                || binaryString.Any(i => i != '0' && i != '1'))
            {
                throw new ArgumentException();
            }
            return Convert.ToByte(binaryString, 2);
        }

        /// <summary>
        /// 结构体转为byte数组
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="obj"></param>
        /// <returns></returns>
        public static byte[] StructToBytes<T>(T obj) //where T : struct
        {
            int size = Marshal.SizeOf(typeof(T));
            IntPtr bufferPtr = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.StructureToPtr(obj, bufferPtr, false);
                byte[] bytes = new byte[size];
                Marshal.Copy(bufferPtr, bytes, 0, size);
                return bytes;
            }
            catch (Exception ex)
            {
                throw ex;
            }
            finally
            {
                Marshal.FreeHGlobal(bufferPtr);
            }
        }

        /// <summary>
        /// byte数组转化为结构体
        /// </summary>
        /// <param name="bytes"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static T BytesToStuct<T>(byte[] bytes)// where T : struct
        {
            Type type = typeof(T);
            int size = Marshal.SizeOf(type);
            if (size > bytes.Length)
            {
                return default;
            }
            IntPtr structPtr = Marshal.AllocHGlobal(size);
            Marshal.Copy(bytes, 0, structPtr, size);
            T t = (T)Marshal.PtrToStructure(structPtr, type);
            Marshal.FreeHGlobal(structPtr);
            return t;
        }


        public static byte[] ClassToBytes<T>(object obj) where T : class
        {
            int res = Marshal.SizeOf(typeof(T));
            byte[] bytes = new byte[res];
            IntPtr buff = Marshal.AllocHGlobal(res);
            Marshal.StructureToPtr(obj, buff, true);
            Marshal.Copy(buff, bytes, 0, res);
            Marshal.FreeHGlobal(buff);

            return bytes;
        }




        /// <summary>
        /// 大小端转换
        /// </summary>
        /// <param name="i"></param>
        /// <returns></returns>
        public static int ReverseInt(int i)
        {
            var ibytes = BitConverter.GetBytes(i);
            var revertByteList = ibytes.Reverse().ToArray();
            return BitConverter.ToInt32(revertByteList, 0);
        }



    }
}
