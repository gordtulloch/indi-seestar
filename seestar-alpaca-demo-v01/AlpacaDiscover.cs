using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using ASCOM.Alpaca.Discovery;
using ASCOM.Common;
using ASCOM.Common.Alpaca;

namespace Seestar_alpaca_demo
{
    public static class DiscoveryHelpers
    {
        // 异步方法：按需筛选设备类型，返回 List<AscomDevice>
        public static async Task<List<AscomDevice>> DiscoverAscomDevicesAsync(DeviceTypes? deviceType = null, double discoveryTimeSeconds = 2.0)
        {
            AlpacaDiscovery discovery = new();
            discovery.StartDiscovery(1, 100, 32227, discoveryTimeSeconds, false, true, false, ServiceType.Http);

            // 等待发现完成（参考 AlpacaDiscoveryTests.cs 的等待方式）
            await Task.Run(() =>
            {
                do
                {
                    Thread.Sleep(50);
                } while (!discovery.DiscoveryComplete);
            });

            return discovery.GetAscomDevices(deviceType);
        }
    }
}