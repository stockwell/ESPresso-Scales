#!/usr/bin/env python3

import asyncio
import aiohttp
from aiohttp import web
import socket
import uuid
import os

from rich.progress import (
    BarColumn,
    DownloadColumn,
    Progress,
    TaskID,
    TextColumn,
    TimeRemainingColumn,
    TransferSpeedColumn,
)

update_file = '../Build/ESPresso-Scales.bin'

def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(0)
    try:
        s.connect(('1.1.1.1', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP


async def initiate():
    expected_size = os.path.getsize(update_file)

    with Progress(
            TextColumn("[bold blue]{task.fields[filename]}", justify="right"),
            BarColumn(),
            "[progress.percentage]{task.percentage:>3.1f}%",
            "|",
            DownloadColumn(),
            "|",
            TransferSpeedColumn(),
            "|",
            TimeRemainingColumn()) as progress:

        base_url = "http://espresso-scales.local/api/v1/update/"

        request = {
            'URL': 'http://' + str(get_ip()) + ':8080/update.bin',
            'UUID': str(uuid.uuid4()),
            'filesize': expected_size,
        }

        progress.print(f"Initiating update from {request['URL']} with UUID: {request['UUID']}...")
        runner = await create_server()

        try:
            timeout = aiohttp.ClientTimeout(total=None, connect=15)
            async with aiohttp.ClientSession(timeout=timeout) as session:
                async with session.post(base_url + 'initiate', json=request) as resp:
                    task_id = progress.add_task("Updating", filename="update.bin", total=expected_size)
                    while True:
                        async with session.get(base_url + 'status') as resp:
                            response = await resp.json()
                            completed = response['progress']
                            progress.update(task_id, completed=completed)

                            if completed == expected_size:
                                break

                        await asyncio.sleep(0.1)

        except Exception as e:
            print(e)
            progress.print("Update timed out")
            pass

        await runner.cleanup()
        return


async def update(request):
    return web.FileResponse(update_file)


async def shutdown(server, app):
    server.close()
    await server.wait_closed()
    await app.shutdown()
    await app.cleanup()


async def create_server():
    app = web.Application()
    app.add_routes([web.get('/update.bin', update)])

    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, '0.0.0.0', 8080)
    await site.start()

    return runner

async def cleanup_server(runner):
    await runner.cleanup()

loop = asyncio.get_event_loop()

try:
    loop.run_until_complete(initiate())
finally:
    loop.close()
