"""Archive read-only battle details and replays without leaking API credentials."""
import argparse
import json
from pathlib import Path
import urllib.request
import urllib.error
from urllib.parse import urljoin, urlparse
from unswbc import api, auth


class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        return None


def download(path, key):
    url = api.url(path)
    origin = urlparse(url)
    opener = urllib.request.build_opener(NoRedirect)
    for _ in range(6):
        parsed = urlparse(url)
        if parsed.scheme != 'https':
            raise RuntimeError('Refusing non-HTTPS download')
        headers = {'User-Agent': 'battlecode-replay-audit'}
        if (parsed.scheme, parsed.netloc) == (origin.scheme, origin.netloc):
            headers['Authorization'] = f'Bearer {key}'
        try:
            with opener.open(urllib.request.Request(url, headers=headers), timeout=120) as reply:
                return reply.read()
        except urllib.error.HTTPError as e:
            if e.code not in (301, 302, 303, 307, 308):
                raise RuntimeError(f'Download failed: HTTP {e.code}') from None
            url = urljoin(url, e.headers['Location'])
    raise RuntimeError('Too many redirects')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('battle_ids', type=int, nargs='+')
    args = parser.parse_args()
    key = auth.key()
    root = Path('evaluations/online')
    root.mkdir(parents=True, exist_ok=True)
    for battle_id in args.battle_ids:
        details = api.request(f'battles/{battle_id}', key=key)
        (root / f'{battle_id}.json').write_text(json.dumps(details, indent=2) + '\n')
        print(f'{battle_id}: saved details', flush=True)
        data = download(f'battles/{battle_id}/replay', key)
        (root / f'{battle_id}.replay').write_bytes(data)
        print(f'{battle_id}: saved replay ({len(data)} bytes)', flush=True)


if __name__ == '__main__':
    main()
