"""Live desktop smoke test. Briefly pauses input and plays two preview meows."""
import json
import subprocess


def call(method, *args):
    return subprocess.check_output([
        'qdbus6', 'local.clickmeow.App', '/ClickMeow',
        'local.clickmeow.App.' + method, *args,
    ], text=True).strip()


def status():
    return json.loads(call('Status'))


initial = status()
assert initial['ready'] == initial['sounds'] >= 11, initial
try:
    call('SetEnabled', 'false')
    paused = status()
    assert not paused['enabled'] and not paused['listenerLoaded'], paused
    call('LeftPressed')
    assert status()['plays'] == paused['plays'], 'Paused clicks must be silent'
    call('PlayTest')
    first = status()
    call('PlayTest')
    second = status()
    assert first['last'] != second['last'], 'Consecutive random sounds repeated'
    assert second['plays'] == paused['plays'] + 2
finally:
    call('SetEnabled', str(initial['enabled']).lower())
final = status()
assert final['enabled'] == initial['enabled']
if initial['enabled']:
    assert final['listenerLoaded'] and not final['error'], final
print('PASS: sounds ready; paused clicks silent; previews do not repeat; original state restored')
print(json.dumps(final, ensure_ascii=False))
