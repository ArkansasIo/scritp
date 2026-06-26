# Unreal Engine 5.8 — Remote Connection Setup Guide

> **PC:** `C:\Users\Shadow`  
> **Target Engine:** Unreal Engine 5.8  
> **Purpose:** Connect from this development machine to a running UE5.8 editor or packaged instance.

---

## 1. Remote Control API (HTTP)

Best for automation, web dashboards, and tooling integration.

### On the UE machine

1. **Enable the plugin:**
   - Edit > Plugins > Built-in > Remote Control > **Enabled**
   - Restart the editor.

2. **Configure remote control:**
   - Edit > Project Settings > Plugins > Remote Control
   - Enable HTTP Server
   - Set **HTTP Port** (default `30010`)
   - Optionally enable WebSocket Server (default `30020`)

3. **Expose properties/functions:**
   - Window > Remote Control > Web App > **Manage Exposed Controllers**
   - Add the actors, properties, or functions you want accessible.

4. **Firewall:** Allow inbound on the chosen ports (`30010`/`30020`).

### From this PC

```powershell
# Test connection (replace IP with UE machine's IP)
curl http://<UE_IP>:30010/remote/object?objectPath=/Script/Engine.Default__Actor
```

**Common endpoints:**
- `GET /remote/object` — read property values
- `PUT /remote/object` — set property values
- `PUT /remote/object/call` — call functions
- `GET /remote/search` — search for objects

---

## 2. Pixel Streaming (WebRTC)

Best for high-fidelity interactive streaming to a browser.

### On the UE machine

1. **Enable the plugin:**
   - Edit > Plugins > Built-in > Graphics > Pixel Streaming > **Enabled**
   - Restart.

2. **Configure:**
   - Edit > Project Settings > Plugins > Pixel Streaming
   - **Signalling Server Port:** `8888`
   - **Streamer Port:** `8889`

3. **Launch the signalling server:**
   ```powershell
   # From the UE engine directory
   .\Engine\Source\Programs\PixelStreaming\WebServers\SignallingWebServer\run.bat
   ```

4. **Start streaming:**
   - Window > Pixel Streaming > Start Signalling Server
   - Window > Pixel Streaming > Start Streamer

5. **Firewall:** Allow ports `8888` (WebRTC) and `80`/`443` (browser frontend).

### From this PC

Open a browser to:
```
http://<UE_IP>:8888
```

---

## 3. Python Remote Execution

Best for scripting and editor automation.

### On the UE machine

1. **Enable the plugin:**
   - Edit > Plugins > Built-in > Scripting > Python Editor Script Plugin > **Enabled**
   - Restart.

2. **Enable remote execution:**
   - Edit > Project Settings > Plugins > Python Editor Script Plugin
   - Enable **Remote Execution**
   - Set **Remote Execution Port** (default `30030`)

3. **Firewall:** Allow port `30030`.

### From this PC

```powershell
# Using the Python remote executor
python -m unreal.RemoteExecutor --host <UE_IP> --port 30030
```

Or with curl:
```powershell
curl -X POST http://<UE_IP>:30030/execute `
  -H "Content-Type: application/json" `
  -d '{"command": "print(unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors())"}'
```

---

## 4. Live Coding / Hot Reload

Best for iterative C++ development.

### Setup

1. **On the UE machine:**
   - Edit > Editor Preferences > General > Live Coding > **Enable Live Coding**
   - Or launch with `-LiveCoding` command-line arg.

2. **Connection:** Live Coding uses named pipes (Windows) — no network config needed. It works automatically when the project is compiled from Visual Studio or Rider on the **same machine**.

> **Note:** Live Coding does not work across separate machines. For remote C++ iteration, use the Remote Control API to trigger recompiles or use network shares + symlinks.

---

## 5. Multi-User Editing

Best for collaborative level editing.

### On each UE machine

1. Enable the plugin:
   - Edit > Plugins > Built-in > Editor > Multi-User Editing > **Enabled**

2. **Configure session:**
   - Window > Multi-User > **Create Session**
   - Set session name
   - Note the **Unicast Endpoint** port (default `9030`)

3. **Firewall:** Allow inbound UDP on `9030`–`9040`.

### Joining from this PC

- Window > Multi-User > **Join Session**
- Sessions appear automatically via UDP discovery; or enter the host IP manually.

---

## 6. Quick Network Connectivity Check

```powershell
# Test basic connectivity
Test-NetConnection -ComputerName <UE_IP> -Port 30010

# Find the UE machine's IP on the engine side
# (Run on the UE machine)
ipconfig
```

---

## Port Reference

| Service                | Default Port | Protocol |
|------------------------|-------------|----------|
| Remote Control HTTP    | 30010       | TCP      |
| Remote Control WebSocket | 30020     | TCP      |
| Python Remote Exec     | 30030       | TCP      |
| Pixel Streaming (HTTP) | 8888        | TCP      |
| Pixel Streaming (Data) | 8889        | TCP      |
| Multi-User Editing     | 9030–9040   | UDP      |

---

## Troubleshooting

| Symptom | Check |
|---------|-------|
| Connection refused | Port blocked by firewall; service not running |
| Timeout | Wrong IP address; network isolation (different VLAN/subnet) |
| Auth required | Enable authentication in project settings; add token to requests |
| Pixel Streaming blank | WebRTC requires HTTPS in modern browsers; use `--allow-running-insecure-content` flag |

---

## References

- [UE Remote Control API Docs](https://docs.unrealengine.com/5.3/en-US/remote-control-api/)
- [UE Pixel Streaming Docs](https://docs.unrealengine.com/5.3/en-US/pixel-streaming-in-unreal-engine/)
- [UE Python Scripting](https://docs.unrealengine.com/5.3/en-US/python-scripting/)
