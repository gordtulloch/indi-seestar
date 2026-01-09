using ASCOM.Alpaca.Discovery;
using ASCOM.Common;
using ASCOM.Common.DeviceInterfaces;

namespace alpaca_alpaca_demo
{
    public partial class Form1 : Form
    {
        private ASCOM.Alpaca.Clients.AlpacaCamera? _cameraClient = null;
        private ASCOM.Alpaca.Clients.AlpacaFocuser? _focuserClient = null;
        private ASCOM.Alpaca.Clients.AlpacaFilterWheel? _filterWheelClient = null;
        private ASCOM.Alpaca.Clients.AlpacaTelescope? _telescopeClient = null;
        private AscomDevice? _cameraServer = null;
        private AscomDevice? _focuserServer = null;
        private AscomDevice? _filterWheelServer = null;
        private AscomDevice? _telescopeServer = null;
        private System.Windows.Forms.Timer? _telescopeUpdateTimer = null;

        public Form1()
        {
            InitializeComponent();
            FocuserUIEnable(false);
            FocuserUIRefresh(false);
            CameraUIEnable(false);
            CameraUIRefresh(false);
            FilterUIEnable(false);
            FilterUIRefresh(false);
            TelescopeUIEnable(false);
            TelescopeUIRefresh(false);
            // 软件打开后自动触发一次发现
            this.Shown += Form1_Shown;
        }
        private void Form1_Shown(object? sender, EventArgs e)
        {
            // 只运行一次，避免因重新显示窗体导致重复触发
            this.Shown -= Form1_Shown;
            // 直接复用按钮点击逻辑(异步,不会阻塞UI)
            btnCameraDiscover_Click(btnCameraDiscover, EventArgs.Empty);
            btnFocuserDiscover_Click(btnFocuserDiscover, EventArgs.Empty);
            btnFilterDiscover_Click(btnFilterDiscover, EventArgs.Empty);
            btnTelescopeDiscover_Click(btnTelescopeDiscover, EventArgs.Empty);
        }

        private void CameraUIEnable(bool enable)
        {
            labelGain.Enabled = enable;
            numericUpDownGain.Enabled = enable;
            labelExp.Enabled = enable;
            labelExp2.Enabled = enable;
            numericUpDownExpSec.Enabled = enable;
            btnStartExp.Enabled = enable;
        }

        private void CameraUIRefresh(bool connect)
        {
            if (connect)
            {
                labelCameraConnectState.Text = "Connected";
                labelCameraName.Text = _cameraServer?.AscomDeviceName ?? "Unknown";
                //labelCameraName.Text = _cameraClient.Name;
                labelCameraDescription.Text = _cameraClient?.Description ?? "Unknown";
                labelCameraDriverInfo.Text = _cameraClient.DriverInfo;
                labelCameraDriverVersion.Text = _cameraClient.DriverVersion;
                labelSensorType.Text = _cameraClient.SensorType.ToString();
                labelSensorXSize.Text = _cameraClient.CameraXSize.ToString();
                labelSensorYSize.Text = _cameraClient.CameraYSize.ToString();
                labelMinExpSec.Text = _cameraClient.ExposureMin.ToString();
                labelMaxExpSec.Text = _cameraClient.ExposureMax.ToString();
                labelMaxBinX.Text = _cameraClient.MaxBinX.ToString();
                labelMaxBinY.Text = _cameraClient.MaxBinY.ToString();
                labelPixelSizeX.Text = _cameraClient.PixelSizeX.ToString();
                labelPixelSizeY.Text = _cameraClient.PixelSizeY.ToString();
                labelMinGain.Text = _cameraClient.GainMin.ToString();
                labelMaxGain.Text = _cameraClient.GainMax.ToString();
                labelSensorTemperature.Text = _cameraClient.CCDTemperature.ToString();
                labelCameraState.Text = _cameraClient.CameraState.ToString();
                labelImageReady.Text = _cameraClient.ImageReady.ToString();

                numericUpDownExpSec.Minimum = (decimal)_cameraClient.ExposureMin;
                numericUpDownExpSec.Maximum = (decimal)_cameraClient.ExposureMax;
                numericUpDownGain.Minimum = _cameraClient.GainMin;
                numericUpDownGain.Maximum = _cameraClient.GainMax;
            }
            else
            {
                labelCameraConnectState.Text = "Disconnected";
                labelCameraName.Text = "xxx";
                labelCameraDescription.Text = "xxx";
                labelCameraDriverInfo.Text = "xxx";
                labelCameraDriverVersion.Text = "xxx";
                labelSensorType.Text = "xxx";
                labelSensorXSize.Text = "xxx";
                labelSensorYSize.Text = "xxx";
                labelMinExpSec.Text = "xxx";
                labelMaxExpSec.Text = "xxx";
                labelMaxBinX.Text = "xxx";
                labelMaxBinY.Text = "xxx";
                labelPixelSizeX.Text = "xxx";
                labelPixelSizeY.Text = "xxx";
                labelMinGain.Text = "xxx";
                labelMaxGain.Text = "xxx";
                labelSensorTemperature.Text = "xxx";
                labelCameraState.Text = "xxx";
                labelImageReady.Text = "xxx";
            }



        }
        public void AppendLog(string message, DeviceTypes? deviceType = null)
        {
            TextBox targetTextBox = textBoxCameraInfo;

            if (deviceType.HasValue)
            {
                switch (deviceType.Value)
                {
                    case DeviceTypes.Focuser:
                        targetTextBox = textBoxFocuserInfo;
                        break;
                    case DeviceTypes.FilterWheel:
                        targetTextBox = textBoxFilterInfo;
                        break;
                    case DeviceTypes.Telescope:
                        targetTextBox = textBox1;
                        break;
                    default:
                        targetTextBox = textBoxCameraInfo;
                        break;
                }
            }

            targetTextBox.AppendText(message + Environment.NewLine);
        }


        #region Page Camera
        private async void btnCameraDiscover_Click(object sender, EventArgs e)
        {
            await DiscoverDevicesAsync(
                deviceType: DeviceTypes.Camera);
        }
        private async void btnCameraConnect_Click(object sender, EventArgs e)
        {
            await ToggleDeviceConnectionAsync(
                deviceType: DeviceTypes.Camera);
        }
        private async void btnStartExp_Click(object sender, EventArgs e)
        {
            btnStartExp.Enabled = false;
            labelCameraState.Text = "Idle";
            labelImageReady.Text = false.ToString();

            // 在 C# 中，decimal 是 定点十进制浮点类型，它能精确表示大多数常见的十进制数（不像 float/double 存在二进制精度误差）。
            var val = numericUpDownExpSec.Value;
            if (val.Equals(0m))
            {
                val = 32 / 1000_000m;
            }
            _cameraClient?.StartExposure((double)val, true);
            AppendLog($"StartExposureAsync success", DeviceTypes.Camera);
            AppendLog($"Exposuring...", DeviceTypes.Camera);

            // 启动简单轮询，刷新相机状态与是否就绪
            await StartExposurePolling();
            btnStartExp.Enabled = true;
        }
        private async Task StartExposurePolling()
        {
            while (_cameraClient != null)
            {
                var state = _cameraClient.CameraState;
                var ready = _cameraClient.ImageReady;

                labelCameraState.Text = state.ToString();
                labelImageReady.Text = ready.ToString();

                if (ready)
                {
                    AppendLog("server image ready", DeviceTypes.Camera);

                    var camera = _cameraClient;
                    if (camera == null)
                    {
                        AppendLog("相机客户端已断开", DeviceTypes.Camera);
                        break;
                    }

                    labelCameraState.Text = camera.CameraState.ToString();

                    try
                    {
                        AppendLog("downloading image...", DeviceTypes.Camera);
                        var bitmap = await Task.Run(() =>
                        {
                            var imgArray = (Array)camera.ImageArray; // 2D 数组（类型可能为 int/short/double 等）
                            return ImageProcessingHelper.ConvertImageArrayToBitmap(imgArray);
                        });
                        AppendLog("image downloaded", DeviceTypes.Camera);

                        if (pictureBox.Image != null)
                        {
                            var old = pictureBox.Image;
                            pictureBox.Image = null;
                            old.Dispose();
                        }

                        pictureBox.Image = bitmap;
                        pictureBox.SizeMode = PictureBoxSizeMode.Zoom;
                        AppendLog("image displayed", DeviceTypes.Camera);
                    }
                    catch (Exception ex)
                    {
                        AppendLog($"显示图像失败: {ex.Message}", DeviceTypes.Camera);
                    }
                    break;
                }
                if (state == CameraState.Error)
                {
                    AppendLog("exp failed", DeviceTypes.Camera);
                    break;
                }

                await Task.Delay(10);
            }
        }

        private void numericUpDownGain_ValueChanged(object sender, EventArgs e)
        {
            _cameraClient.Gain = (short)numericUpDownGain.Value;
        }
        #endregion

        #region Page Focuser
        private async void btnFocuserDiscover_Click(object sender, EventArgs e)
        {
            await DiscoverDevicesAsync(
                deviceType: DeviceTypes.Focuser);
        }
        private void FocuserUIEnable(bool connected)
        {
            labelFocuserTargetPosText.Enabled = connected;
            numericUpDownFocuserTargetPosition.Enabled = connected;
            btnFocuserMove.Enabled = connected;
        }
        private void FocuserUIRefresh(bool connected)
        {
            if (connected)
            {
                labelFocuserConnectState.Text = "Connected";
                labelFocuserName.Text = _focuserServer.AscomDeviceName;
                labelFocuserDescription.Text = _focuserClient.Description;
                labelFocuserDriverInfo.Text = _focuserClient.DriverInfo;
                labelFocuserDriverVersion.Text = _focuserClient.DriverVersion;
                labelFocuserIsMoving.Text = _focuserClient.IsMoving.ToString();
                labelFocuserMaxIncrement.Text = _focuserClient.MaxIncrement.ToString();
                labelFocuserMaxStep.Text = _focuserClient.MaxStep.ToString();
                labelFocuserPosition.Text = _focuserClient.Position.ToString();
                labelFocuserTemperature.Text = _focuserClient.Temperature.ToString();

                numericUpDownFocuserTargetPosition.Minimum = 0;
                numericUpDownFocuserTargetPosition.Maximum = _focuserClient.MaxStep;
                numericUpDownFocuserTargetPosition.Value = _focuserClient.Position;
            }
            else
            {
                labelFocuserConnectState.Text = "Disconnected";
                labelFocuserName.Text = "xxx";
                labelFocuserDescription.Text = "xxx";
                labelFocuserDriverInfo.Text = "xxx";
                labelFocuserDriverVersion.Text = "xxx";
                labelFocuserIsMoving.Text = "xxx";
                labelFocuserMaxIncrement.Text = "xxx";
                labelFocuserMaxStep.Text = "xxx";
                labelFocuserPosition.Text = "xxx";
                labelFocuserTemperature.Text = "xxx";
            }
        }

        private async void btnFocuserConnect_Click(object sender, EventArgs e)
        {
            await ToggleDeviceConnectionAsync(
                deviceType: DeviceTypes.Focuser);
        }
        private async void btnFocuserMove_Click(object sender, EventArgs e)
        {
            if (_focuserClient == null)
            {
                AppendLog("当前没有连接的调焦器", DeviceTypes.Focuser);
                return;
            }

            // 如果当前正在移动，则尝试执行 Halt
            if (_focuserClient.IsMoving)
            {
                try
                {
                    _focuserClient.Halt();
                    AppendLog("已发送调焦停止指令", DeviceTypes.Focuser);
                }
                catch (Exception ex)
                {
                    AppendLog($"停止调焦失败: {ex.Message}", DeviceTypes.Focuser);
                }

                btnFocuserMove.Text = "Move";
                return;
            }

            var pos = (int)numericUpDownFocuserTargetPosition.Value;

            try
            {
                _focuserClient.Move(pos);
                AppendLog($"开始调焦移动至 {pos}", DeviceTypes.Focuser);
            }
            catch (Exception ex)
            {
                AppendLog($"调焦移动失败: {ex.Message}", DeviceTypes.Focuser);
                return;
            }

            btnFocuserMove.Text = "Halt";

            try
            {
                while (true)
                {
                    labelFocuserPosition.Text = _focuserClient.Position.ToString();
                    labelFocuserIsMoving.Text = _focuserClient.IsMoving.ToString();

                    if (!_focuserClient.IsMoving)
                        break;

                    await Task.Delay(200);
                }

                labelFocuserPosition.Text = _focuserClient.Position.ToString();
                labelFocuserIsMoving.Text = _focuserClient.IsMoving.ToString();
            }
            catch (TaskCanceledException)
            {
            }
            finally
            {
                btnFocuserMove.Text = "Move";
            }
        }
        #endregion

        #region Page Filter Wheel
        private void FilterUIEnable(bool connected)
        {
            comboBoxFilters.Enabled = connected;
            btnFilterChange.Enabled = connected;
        }

        private void FilterUIRefresh(bool connected)
        {
            if (connected)
            {
                labelFilterConnectState.Text = "Connected";
                labelFilterName.Text = _filterWheelServer.AscomDeviceName;
                labelFilterDescription.Text = _filterWheelClient.Description;
                labelFilterDriverInfo.Text = _filterWheelClient.DriverInfo;
                labelFilterDriverVersion.Text = _filterWheelClient.DriverVersion;
                labelFilterPosition.Text = _filterWheelClient.Position.ToString();

                var filterNames = _filterWheelClient.Names;
                comboBoxFilters.Items.Clear();
                if (filterNames != null)
                {
                    comboBoxFilters.Items.AddRange(filterNames);
                }

                if (_filterWheelClient.Position >= 0 && _filterWheelClient.Position < comboBoxFilters.Items.Count)
                {
                    comboBoxFilters.SelectedIndex = _filterWheelClient.Position;
                }

            }
            else
            {
                labelFilterConnectState.Text = "Disconnected";
                labelFilterName.Text = "xxx";
                labelFilterDescription.Text = "xxx";
                labelFilterDriverInfo.Text = "xxx";
                labelFilterDriverVersion.Text = "xxx";
                labelFilterPosition.Text = "xxx";
                comboBoxFilters.SelectedIndex = -1;
                comboBoxFilters.Items.Clear();
                //textBoxFilterInfo.Clear();
            }
        }

        private async void btnChangeFilter_Click(object sender, EventArgs e)
        {
            try
            {
                btnFilterChange.Enabled = false;
                comboBoxFilters.Enabled = false;
                var targetSlot = comboBoxFilters.SelectedIndex;
                _filterWheelClient.Position = (short)targetSlot;
                //AppendLog($"正在切换滤镜到位置 {targetSlot}", DeviceTypes.FilterWheel);

                await Task.Run(async () =>
                {
                    while (true)
                    {
                        if (_filterWheelClient.Position == targetSlot)
                            break;

                        await Task.Delay(200);
                    }
                });
                labelFilterPosition.Text = _filterWheelClient.Position.ToString();
                //FilterUIRefresh(true);
                AppendLog($"切换滤镜{comboBoxFilters.Text}完成", DeviceTypes.FilterWheel);

            }
            catch (Exception ex)
            {
                AppendLog($"切换滤镜失败: {ex.Message}", DeviceTypes.FilterWheel);
            }
            finally
            {
                btnFilterChange.Enabled = true;
                comboBoxFilters.Enabled = true;
            }
        }
        #endregion


        #region common methods

        private async Task DisconnectDeviceAsync(DeviceTypes deviceType)
        {
            switch (deviceType)
            {
                case DeviceTypes.Focuser:
                    if (_focuserServer == null || _focuserClient == null)
                    {
                        AppendLog("当前没有连接的调焦器", DeviceTypes.Focuser);
                        return;
                    }

                    await _focuserClient.DisconnectAsync(DeviceTypes.Focuser, _focuserClient.InterfaceVersion);
                    AppendLog($"断开连接成功: {_focuserServer.AscomDeviceName} @ {_focuserServer.IpAddress}:{_focuserServer.IpPort} #{_focuserServer.AlpacaDeviceNumber}", DeviceTypes.Focuser);
                    _focuserClient = null;
                    return;

                case DeviceTypes.Camera:
                    if (_cameraServer == null || _cameraClient == null)
                    {
                        AppendLog("当前没有连接的相机", DeviceTypes.Camera);
                        return;
                    }

                    await _cameraClient.DisconnectAsync(DeviceTypes.Camera, _cameraClient.InterfaceVersion);
                    AppendLog($"断开连接成功: {_cameraServer.AscomDeviceName} @ {_cameraServer.IpAddress}:{_cameraServer.IpPort} #{_cameraServer.AlpacaDeviceNumber}", DeviceTypes.Camera);
                    _cameraClient = null;
                    return;

                case DeviceTypes.FilterWheel:
                    if (_filterWheelServer == null || _filterWheelClient == null)
                    {
                        AppendLog("当前没有连接的滤镜轮", DeviceTypes.FilterWheel);
                        return;
                    }

                    await _filterWheelClient.DisconnectAsync(DeviceTypes.FilterWheel, _filterWheelClient.InterfaceVersion);
                    AppendLog($"断开连接成功: {_filterWheelServer.AscomDeviceName} @ {_filterWheelServer.IpAddress}:{_filterWheelServer.IpPort} #{_filterWheelServer.AlpacaDeviceNumber}", DeviceTypes.FilterWheel);
                    _filterWheelClient = null;
                    return;

                case DeviceTypes.Telescope:
                    if (_telescopeServer == null || _telescopeClient == null)
                    {
                        AppendLog("当前没有连接的望远镜", DeviceTypes.Telescope);
                        return;
                    }

                    await _telescopeClient.DisconnectAsync(DeviceTypes.Telescope, _telescopeClient.InterfaceVersion);
                    AppendLog($"断开连接成功: {_telescopeServer.AscomDeviceName} @ {_telescopeServer.IpAddress}:{_telescopeServer.IpPort} #{_telescopeServer.AlpacaDeviceNumber}", DeviceTypes.Telescope);
                    _telescopeClient = null;
                    return;

                default:
                    return;
            }
        }

        private async Task ConnectDeviceAsync(DeviceTypes deviceType, AscomDevice device)
        {
            switch (deviceType)
            {
                case DeviceTypes.Focuser:
                    _focuserClient = ASCOM.Alpaca.Clients.AlpacaClient.GetDevice<ASCOM.Alpaca.Clients.AlpacaFocuser>(device);
                    await _focuserClient.ConnectAsync(DeviceTypes.Focuser, _focuserClient.InterfaceVersion);
                    _focuserServer = device;
                    AppendLog($"连接成功: {device.AscomDeviceName} @ {device.IpAddress}:{device.IpPort} #{device.AlpacaDeviceNumber}", DeviceTypes.Focuser);
                    return;

                case DeviceTypes.Camera:
                    _cameraClient = ASCOM.Alpaca.Clients.AlpacaClient.GetDevice<ASCOM.Alpaca.Clients.AlpacaCamera>(device);
                    await _cameraClient.ConnectAsync(DeviceTypes.Camera, _cameraClient.InterfaceVersion);
                    _cameraServer = device;
                    AppendLog($"连接成功: {device.AscomDeviceName} @ {device.IpAddress}:{device.IpPort} #{device.AlpacaDeviceNumber}", DeviceTypes.Camera);
                    return;

                case DeviceTypes.FilterWheel:
                    _filterWheelClient = ASCOM.Alpaca.Clients.AlpacaClient.GetDevice<ASCOM.Alpaca.Clients.AlpacaFilterWheel>(device);
                    await _filterWheelClient.ConnectAsync(DeviceTypes.FilterWheel, _filterWheelClient.InterfaceVersion);
                    _filterWheelServer = device;
                    AppendLog($"连接成功: {device.AscomDeviceName} @ {device.IpAddress}:{device.IpPort} #{device.AlpacaDeviceNumber}", DeviceTypes.FilterWheel);
                    return;

                case DeviceTypes.Telescope:
                    // NOTE: ASCOM.Alpaca Client Library 2.2.1 版本用此方法，因为Client Library有问题
                    _telescopeClient = new ASCOM.Alpaca.Clients.AlpacaTelescope(
                        device.ServiceType,
                        device.IpAddress,
                        device.IpPort,
                        device.AlpacaDeviceNumber,
                        strictCasing: false,
                        logger: null);
                    // NOTE: ASCOM.Alpaca Client Library 2.1.0版本用此方法，和其他设备统一
                    //_telescopeClient = ASCOM.Alpaca.Clients.AlpacaClient.GetDevice<ASCOM.Alpaca.Clients.AlpacaTelescope>(device);
                    await _telescopeClient.ConnectAsync(DeviceTypes.Telescope, _telescopeClient.InterfaceVersion);
                    _telescopeServer = device;
                    AppendLog($"连接成功: {device.AscomDeviceName} @ {device.IpAddress}:{device.IpPort} #{device.AlpacaDeviceNumber}", DeviceTypes.Telescope);
                    return;

                default:
                    return;
            }
        }
        private void OnDeviceConnected(DeviceTypes deviceType)
        {
            switch (deviceType)
            {
                case DeviceTypes.Focuser:
                    FocuserUIEnable(true);
                    FocuserUIRefresh(true);
                    return;

                case DeviceTypes.Camera:
                    CameraUIEnable(true);
                    CameraUIRefresh(true);
                    return;

                case DeviceTypes.FilterWheel:
                    FilterUIEnable(true);
                    FilterUIRefresh(true);
                    return;

                case DeviceTypes.Telescope:
                    TelescopeUIEnable(true);
                    TelescopeUIRefresh(true);
                    return;

                default:
                    return;
            }
        }

        private void OnDeviceDisconnected(DeviceTypes deviceType)
        {
            switch (deviceType)
            {
                case DeviceTypes.Focuser:
                    _focuserServer = null;
                    FocuserUIEnable(false);
                    FocuserUIRefresh(false);
                    return;

                case DeviceTypes.Camera:
                    _cameraServer = null;
                    CameraUIEnable(false);
                    CameraUIRefresh(false);
                    return;

                case DeviceTypes.FilterWheel:
                    _filterWheelServer = null;
                    FilterUIEnable(false);
                    FilterUIRefresh(false);
                    return;

                case DeviceTypes.Telescope:
                    _telescopeServer = null;
                    TelescopeUIEnable(false);
                    TelescopeUIRefresh(false);
                    return;

                default:
                    return;
            }
        }

        private sealed class DeviceInfo
        {
            public string DisplayName { get; set; } = string.Empty;
            public AscomDevice Device { get; set; } = null!;
        }

        private async Task DiscoverDevicesAsync(DeviceTypes deviceType)
        {
            ComboBox comboBox;
            Button btnDiscover;
            Button btnConnect;

            switch (deviceType)
            {
                case DeviceTypes.Focuser:
                    comboBox = comboBoxFocuserList;
                    btnDiscover = btnFocuserDiscover;
                    btnConnect = btnFocuserConnect;
                    break;
                case DeviceTypes.FilterWheel:
                    comboBox = comboBoxFilterList;
                    btnDiscover = btnFilterDiscover;
                    btnConnect = btnFilterConnect;
                    break;
                case DeviceTypes.Telescope:
                    comboBox = comboBoxTelescopeList;
                    btnDiscover = btnTelescopeDiscover;
                    btnConnect = btnTelescopeConnect;
                    break;
                case DeviceTypes.Camera:
                default:
                    comboBox = comboBoxCameraList;
                    btnDiscover = btnCameraDiscover;
                    btnConnect = btnCameraConnect;
                    break;
            }

            try
            {
                btnDiscover.Enabled = false;
                btnConnect.Enabled = false;
                AppendLog($"开始发现{deviceType}设备...", deviceType);

                var discoveredDevices = await DiscoveryHelpers.DiscoverAscomDevicesAsync(deviceType: deviceType, discoveryTimeSeconds: 2.0);

                List<DeviceInfo> listDevices = new List<DeviceInfo>();
                foreach (var device in discoveredDevices)
                {
                    var displayName = $"{device.AscomDeviceName} @ {device.IpAddress} #{device.AlpacaDeviceNumber}";
                    listDevices.Add(new DeviceInfo
                    {
                        DisplayName = displayName,
                        Device = device
                    });
                }

                comboBox.BeginUpdate();
                comboBox.DataSource = null;
                comboBox.Items.Clear();
                comboBox.DisplayMember = nameof(DeviceInfo.DisplayName);
                comboBox.ValueMember = nameof(DeviceInfo.Device);
                comboBox.DataSource = listDevices;
                comboBox.EndUpdate();

                if (listDevices.Count > 0)
                {
                    comboBox.SelectedIndex = 0;
                    btnConnect.Enabled = true;
                }

                AppendLog($"已发现{deviceType}设备: {listDevices.Count}", deviceType);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"发现{deviceType}失败: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
                btnConnect.Enabled = false;
            }
            finally
            {
                btnDiscover.Enabled = true;
            }
        }

        private async Task ToggleDeviceConnectionAsync(DeviceTypes deviceType)
        {
            ComboBox comboBox;
            Button connectButton;
            bool isConnected = false;
            Func<Task> disconnectAction = () => DisconnectDeviceAsync(deviceType);
            Func<DeviceInfo, Task> connectAction = deviceInfo => ConnectDeviceAsync(deviceType, deviceInfo.Device);
            Action? onConnected = () => OnDeviceConnected(deviceType);
            Action? onDisconnected = () => OnDeviceDisconnected(deviceType);

            switch (deviceType)
            {
                case DeviceTypes.Focuser:
                    comboBox = comboBoxFocuserList;
                    connectButton = btnFocuserConnect;
                    isConnected = _focuserClient != null && _focuserServer != null && _focuserClient.Connected;
                    break;

                case DeviceTypes.FilterWheel:
                    comboBox = comboBoxFilterList;
                    connectButton = btnFilterConnect;
                    isConnected = _filterWheelClient != null && _filterWheelServer != null && _filterWheelClient.Connected;
                    break;

                case DeviceTypes.Telescope:
                    comboBox = comboBoxTelescopeList;
                    connectButton = btnTelescopeConnect;
                    isConnected = _telescopeClient != null && _telescopeServer != null && _telescopeClient.Connected;
                    break;

                case DeviceTypes.Camera:
                default:
                    comboBox = comboBoxCameraList;
                    connectButton = btnCameraConnect;
                    isConnected = _cameraClient != null && _cameraServer != null && _cameraClient.Connected;
                    break;
            }

            try
            {
                connectButton.Enabled = false;

                if (isConnected)
                {
                    await disconnectAction();
                    connectButton.Text = "Connect";
                    onDisconnected?.Invoke();
                    return;
                }

                if (comboBox.SelectedItem is DeviceInfo deviceInfo)
                {
                    await connectAction(deviceInfo);
                    onConnected?.Invoke();
                    connectButton.Text = "Disconnect";
                    return;
                }

                AppendLog($"请先选择{deviceType}设备", deviceType);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"连接{deviceType}失败: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
                connectButton.Enabled = true;
            }
        }
        #endregion

        #region Page Filter
        private async void btnFilterDiscover_Click(object sender, EventArgs e)
        {
            await DiscoverDevicesAsync(
                deviceType: DeviceTypes.FilterWheel);
        }
        private async void btnFilterConnect_Click(object sender, EventArgs e)
        {
            await ToggleDeviceConnectionAsync(
                deviceType: DeviceTypes.FilterWheel);
        }
        #endregion

        #region Page Telescope
        private void TelescopeUIEnable(bool connected)
        {
            btnTelescopeTargetRADecSlew.Enabled = connected;
            textBoxTelescopeTargetRA_h.Enabled = connected;
            textBoxTelescopeTargetRA_m.Enabled = connected;
            textBoxTelescopeTargetRA_s.Enabled = connected;
            textBoxTelescopeTargetDec_d.Enabled = connected;
            textBoxTelescopeTargetDec_m.Enabled = connected;
            textBoxTelescopeTargetDec_s.Enabled = connected;

            btnTelescopeTargetAltAzSlew.Enabled = connected;
            textBoxTelescopeTargetAlt_d.Enabled = connected;
            textBoxTelescopeTargetAlt_m.Enabled = connected;
            textBoxTelescopeTargetAlt_s.Enabled = connected;
            textBoxTelescopeTargetAz_d.Enabled = connected;
            textBoxTelescopeTargetAz_m.Enabled = connected;
            textBoxTelescopeTargetAz_s.Enabled = connected;

            btnTelescopeSetTrackingRate.Enabled = connected;
            comboBoxTelescopeTrackingRate.Enabled = connected;
            btnTelescopeUp.Enabled = connected;
            btnTelescopeDown.Enabled = connected;
            btnTelescopeLeft.Enabled = connected;
            btnTelescopeRight.Enabled = connected;
            btnTelescopeStop.Enabled = connected;
            btnTelescopeHome.Enabled = connected;
            numericUpDownTelescopePrimaryRate.Enabled = connected;
            numericUpDownTelescopeSecondaryRate.Enabled = connected;

            // 启动或停止定时器
            if (connected)
            {
                StartTelescopeUpdateTimer();
            }
            else
            {
                StopTelescopeUpdateTimer();
            }
        }

        private void StartTelescopeUpdateTimer()
        {
            if (_telescopeUpdateTimer == null)
            {
                _telescopeUpdateTimer = new System.Windows.Forms.Timer();
                _telescopeUpdateTimer.Interval = 1000; // 1秒
                _telescopeUpdateTimer.Tick += TelescopeUpdateTimer_Tick;
            }
            _telescopeUpdateTimer.Start();
        }

        private void StopTelescopeUpdateTimer()
        {
            if (_telescopeUpdateTimer != null)
            {
                _telescopeUpdateTimer.Stop();
            }
        }

        private async void TelescopeUpdateTimer_Tick(object? sender, EventArgs e)
        {
            if (_telescopeClient == null || !_telescopeClient.Connected)
            {
                StopTelescopeUpdateTimer();
                return;
            }

            try
            {
                // 保存局部引用,防止在异步操作期间_telescopeClient被设置为null
                var client = _telescopeClient;
                if (client == null)
                {
                    StopTelescopeUpdateTimer();
                    return;
                }

                // 在后台线程获取数据,避免阻塞UI
                var telescopeData = await Task.Run(() =>
                {
                    return new
                    {
                        SiderealTime = client.SiderealTime,
                        RightAscension = client.RightAscension,
                        Declination = client.Declination,
                        Altitude = client.Altitude,
                        Azimuth = client.Azimuth,
                        Tracking = client.Tracking,
                        TrackingRate = client.TrackingRate
                    };
                });

                // 在UI线程更新界面
                labelTelescopeSiderealTime.Text = ASCOM.Tools.Utilities.HoursToHMS(telescopeData.SiderealTime, ":", ":", "", 2);
                labelTelescopeRA.Text = ASCOM.Tools.Utilities.HoursToHMS(telescopeData.RightAscension, ":", ":", "", 2);
                labelTelescopeDec.Text = ASCOM.Tools.Utilities.DegreesToDMS(telescopeData.Declination, "°", "'", "\"", 1);
                labelTelescopeAltitude.Text = ASCOM.Tools.Utilities.DegreesToDMS(telescopeData.Altitude, "°", "'", "\"", 1);
                labelTelescopeAzimuth.Text = ASCOM.Tools.Utilities.DegreesToDMS(telescopeData.Azimuth, "°", "'", "\"", 1);
                if (!telescopeData.Tracking)
                {
                    labelTelescopeTracking.Text = "Stopped";
                }
                else
                {
                    labelTelescopeTracking.Text = telescopeData.TrackingRate.ToString();
                }
            }
            catch (Exception ex)
            {
                AppendLog($"更新望远镜信息失败: {ex.Message}", DeviceTypes.Telescope);
            }
        }

        private void TelescopeUIRefresh(bool connected)
        {
            if (connected)
            {
                labelTelescopeConnectState.Text = "Connected";
                labelTelescopeName.Text = _telescopeServer.AscomDeviceName;
                labelTelescopeDescription.Text = _telescopeClient.Description;
                labelTelescopeDriverInfo.Text = _telescopeClient.DriverInfo;
                labelTelescopeDriverVersion.Text = _telescopeClient.DriverVersion;
                labelTelescopeLatitude.Text = ASCOM.Tools.Utilities.DegreesToDMS(_telescopeClient.SiteLatitude, "°", "'", "\"", 1);
                labelTelescopeLongitude.Text = ASCOM.Tools.Utilities.DegreesToDMS(_telescopeClient.SiteLongitude, "°", "'", "\"", 1);
                labelTelescopeEpoch.Text = _telescopeClient.EquatorialSystem.ToString();
                labelTelescopeSiderealTime.Text = ASCOM.Tools.Utilities.HoursToHMS(_telescopeClient.SiderealTime, ":", ":", "", 2);
                labelTelescopeRA.Text = ASCOM.Tools.Utilities.HoursToHMS(_telescopeClient.RightAscension, ":", ":", "", 2);
                labelTelescopeDec.Text = ASCOM.Tools.Utilities.DegreesToDMS(_telescopeClient.Declination, "°", "'", "\"", 1);
                labelTelescopeAltitude.Text = ASCOM.Tools.Utilities.DegreesToDMS(_telescopeClient.Altitude, "°", "'", "\"", 1);
                labelTelescopeAzimuth.Text = ASCOM.Tools.Utilities.DegreesToDMS(_telescopeClient.Azimuth, "°", "'", "\"", 1);
                if (!_telescopeClient.Tracking)
                {
                    labelTelescopeTracking.Text = "Stopped";
                }
                else
                {
                    labelTelescopeTracking.Text = _telescopeClient.TrackingRate.ToString();
                }

                // 填充跟踪速率下拉框
                comboBoxTelescopeTrackingRate.Items.Clear();
                comboBoxTelescopeTrackingRate.Items.Add(DriveRate.Sidereal);
                comboBoxTelescopeTrackingRate.Items.Add(DriveRate.Lunar);
                comboBoxTelescopeTrackingRate.Items.Add(DriveRate.Solar);
                comboBoxTelescopeTrackingRate.Items.Add(DriveRate.King);
                comboBoxTelescopeTrackingRate.SelectedIndex = 0;
            }
            else
            {
                labelTelescopeConnectState.Text = "Disconnected";
                labelTelescopeName.Text = "xxx";
                labelTelescopeDescription.Text = "xxx";
                labelTelescopeDriverInfo.Text = "xxx";
                labelTelescopeDriverVersion.Text = "xxx";
                labelTelescopeLatitude.Text = "xxx";
                labelTelescopeLongitude.Text = "xxx";
                labelTelescopeEpoch.Text = "xxx";
                labelTelescopeSiderealTime.Text = "xxx";
                labelTelescopeRA.Text = "xxx";
                labelTelescopeDec.Text = "xxx";
                labelTelescopeAltitude.Text = "xxx";
                labelTelescopeAzimuth.Text = "xxx";
                labelTelescopeTracking.Text = "xxx";
                comboBoxTelescopeTrackingRate.Items.Clear();
            }
        }

        private async void btnTelescopeDiscover_Click(object sender, EventArgs e)
        {
            await DiscoverDevicesAsync(
                deviceType: DeviceTypes.Telescope);
        }

        private async void btnTelescopeConnect_Click(object sender, EventArgs e)
        {
            await ToggleDeviceConnectionAsync(
                deviceType: DeviceTypes.Telescope);
        }

        private async void btnTelescopeTargetRADecSlew_Click(object sender, EventArgs e)
        {
            if (_telescopeClient == null)
            {
                AppendLog("当前没有连接的望远镜", DeviceTypes.Telescope);
                return;
            }

            try
            {
                // 解析RA输入 (时:分:秒)
                double h = double.Parse(textBoxTelescopeTargetRA_h.Text);
                double m = double.Parse(textBoxTelescopeTargetRA_m.Text);
                double s = double.Parse(textBoxTelescopeTargetRA_s.Text);
                double ra = h + m / 60.0 + s / 3600.0;

                _telescopeClient.TargetRightAscension = ra;
                AppendLog($"设置目标RA: {ra:F6} 小时", DeviceTypes.Telescope);

                // 解析Dec输入 (度:分:秒)
                double d = double.Parse(textBoxTelescopeTargetDec_d.Text);
                double m2 = double.Parse(textBoxTelescopeTargetDec_m.Text);
                double s2 = double.Parse(textBoxTelescopeTargetDec_s.Text);
                double dec = d + Math.Sign(d) * (m2 / 60.0 + s2 / 3600.0);

                _telescopeClient.TargetDeclination = dec;
                AppendLog($"设置目标Dec: {dec:F6} 度", DeviceTypes.Telescope);

                // 执行Slew
                btnTelescopeTargetRADecSlew.Enabled = false;
                AppendLog("开始Slew到目标位置...", DeviceTypes.Telescope);
                
                await Task.Run(() =>
                {
                    _telescopeClient.SlewToTarget();
                });

                // 等待Slew完成
                await Task.Run(async () =>
                {
                    while (_telescopeClient.Slewing)
                    {
                        await Task.Delay(500);
                    }
                });

                AppendLog("Slew完成", DeviceTypes.Telescope);
            }
            catch (Exception ex)
            {
                AppendLog($"设置目标坐标或Slew失败: {ex.Message}", DeviceTypes.Telescope);
            }
            finally
            {
                btnTelescopeTargetRADecSlew.Enabled = true;
            }
        }

        private async void btnTelescopeTargetDecSlew_Click(object sender, EventArgs e)
        {
            // 此方法已合并到 btnTelescopeTargetRADecSlew_Click
            btnTelescopeTargetRADecSlew_Click(sender, e);
        }

        private void btnTelescopeSetTrackingRate_Click(object sender, EventArgs e)
        {
            if (_telescopeClient == null || comboBoxTelescopeTrackingRate.SelectedItem == null)
            {
                AppendLog("请先连接望远镜并选择跟踪速率", DeviceTypes.Telescope);
                return;
            }

            try
            {
                var rate = (DriveRate)comboBoxTelescopeTrackingRate.SelectedItem;
                AppendLog($"设置跟踪速率: {rate}", DeviceTypes.Telescope);
            }
            catch (Exception ex)
            {
                AppendLog($"设置跟踪速率失败: {ex.Message}", DeviceTypes.Telescope);
            }
        }

        /// <summary>
        /// 通用方法：移动望远镜轴
        /// </summary>
        /// <param name="axis">轴类型</param>
        /// <param name="rate">移动速率</param>
        /// <param name="direction">方向描述</param>
        private void MoveTelescopeAxis(TelescopeAxis axis, double rate, string direction)
        {
            if (_telescopeClient == null)
                return;

            try
            {
                _telescopeClient.MoveAxis(axis, rate);
                AppendLog($"{direction},速率: {rate}", DeviceTypes.Telescope);
            }
            catch (Exception ex)
            {
                AppendLog($"移动失败: {ex.Message}", DeviceTypes.Telescope);
            }
        }

        private void btnTelescopeUp_MouseDown(object sender, MouseEventArgs e)
        {
            double rate = (double)numericUpDownTelescopeSecondaryRate.Value;
            MoveTelescopeAxis(TelescopeAxis.Secondary, rate, "向上移动");
        }

        private void btnTelescopeUp_MouseUp(object sender,EventArgs e)
        {
            MoveTelescopeAxis(TelescopeAxis.Secondary, 0, "向上停止");
        }

        private void btnTelescopeDown_MouseDown(object sender,EventArgs e)
        {
            double rate = -(double)numericUpDownTelescopeSecondaryRate.Value;
            MoveTelescopeAxis(TelescopeAxis.Secondary, rate, "向下移动");
        }

        private void btnTelescopeDown_MouseUp(object sender,EventArgs e)
        {
            MoveTelescopeAxis(TelescopeAxis.Secondary, 0, "向下停止");
        }

        private void btnTelescopeLeft_MouseDown(object sender,EventArgs e)
        {
            double rate = -(double)numericUpDownTelescopePrimaryRate.Value;
            MoveTelescopeAxis(TelescopeAxis.Primary, rate, "向左移动");
        }

        private void btnTelescopeLeft_MouseUp(object sender,EventArgs e)
        {
            MoveTelescopeAxis(TelescopeAxis.Primary, 0, "向左停止");
        }

        private void btnTelescopeRight_MouseDown(object sender,EventArgs e)
        {
            double rate = (double)numericUpDownTelescopePrimaryRate.Value;
            MoveTelescopeAxis(TelescopeAxis.Primary, rate, "向右移动");
        }

        private void btnTelescopeRight_MouseUp(object sender,EventArgs e)
        {
            MoveTelescopeAxis(TelescopeAxis.Primary, 0, "向右停止");
        }

        private void btnTelescopeStop_Click(object sender,EventArgs e)
        {
            if (_telescopeClient == null)
                return;

            try
            {
                _telescopeClient.AbortSlew();
                AppendLog("停止移动", DeviceTypes.Telescope);
            }
            catch (Exception ex)
            {
                AppendLog($"停止失败: {ex.Message}", DeviceTypes.Telescope);
            }
        }

        private async void btnTelescopeHome_Click(object sender, EventArgs e)
        {
            if (_telescopeClient == null)
            {
                AppendLog("当前没有连接的望远镜", DeviceTypes.Telescope);
                return;
            }

            try
            {
                btnTelescopeHome.Enabled = false;
                AppendLog("开始归位...", DeviceTypes.Telescope);
                
                await Task.Run(() =>
                {
                    _telescopeClient.FindHome();
                });

                // 等待归位完成
                await Task.Run(async () =>
                {
                    while (_telescopeClient.Slewing)
                    {
                        await Task.Delay(500);
                    }
                });

                AppendLog("归位完成", DeviceTypes.Telescope);
            }
            catch (Exception ex)
            {
                AppendLog($"归位失败: {ex.Message}", DeviceTypes.Telescope);
            }
            finally
            {
                btnTelescopeHome.Enabled = true;
            }
        }
        #endregion
    }
}
