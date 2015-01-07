# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from telemetry.core import forwarders
from telemetry.core import webpagereplay
from telemetry.core import wpr_modes


# TODO(tonyg): Move webpagereplay.py's guts into this class and
# make ReplayServer subclass LocalServer.


class ReplayServer(object):
  def __init__(self, browser_backend, path, wpr_mode,
               make_javascript_deterministic):
    self._browser_backend = browser_backend
    self._forwarder = None
    self._web_page_replay = None

    wpr_args = browser_backend.browser_options.extra_wpr_args
    if wpr_mode == wpr_modes.WPR_APPEND:
      wpr_args.append('--append')
    elif wpr_mode == wpr_modes.WPR_RECORD:
      wpr_args.append('--record')
    if not make_javascript_deterministic:
      wpr_args.append('--inject_scripts=')
    browser_backend.AddReplayServerOptions(wpr_args)
    self._web_page_replay = webpagereplay.ReplayServer(
        path, self._browser_backend.forwarder_factory.host_ip,
        browser_backend.wpr_port_pairs.dns.local_port if
        browser_backend.wpr_port_pairs.dns else 0,
        browser_backend.wpr_port_pairs.http.local_port,
        browser_backend.wpr_port_pairs.https.local_port,
        wpr_args)
    # Remove --no-dns_forwarding if it wasn't explicitly requested by backend.
    if '--no-dns_forwarding' not in wpr_args:
      self._web_page_replay.replay_options.remove('--no-dns_forwarding')
    self._web_page_replay.StartServer()

    browser_backend.wpr_port_pairs = forwarders.PortPairs(
        http=forwarders.PortPair(
            self._web_page_replay.http_port,
            browser_backend.wpr_port_pairs.http.remote_port or
            self._web_page_replay.http_port),
        https=forwarders.PortPair(
            self._web_page_replay.https_port,
            browser_backend.wpr_port_pairs.https.remote_port or
            self._web_page_replay.https_port),
        dns=forwarders.PortPair(
            self._web_page_replay.dns_port,
            browser_backend.wpr_port_pairs.dns.remote_port or
            self._web_page_replay.dns_port)
            if browser_backend.wpr_port_pairs.dns else None)

    self._forwarder = browser_backend.forwarder_factory.Create(
        browser_backend.wpr_port_pairs)

  def __enter__(self):
    return self

  def __exit__(self, *args):
    self.Close()

  def Close(self):
    if self._forwarder:
      self._forwarder.Close()
      self._forwarder = None
    if self._web_page_replay:
      self._web_page_replay.StopServer()
      self._web_page_replay = None
