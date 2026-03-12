import json
import urllib.request


BASE_URL = "http://localhost:8080"


def _post(path: str, payload: dict) -> dict:
    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(
        BASE_URL + path,
        data=data,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    with urllib.request.urlopen(req) as resp:
        return json.loads(resp.read().decode("utf-8"))


def complete(prompt: str) -> str:
    resp = _post("/v1/complete", {"prompt": prompt})
    return resp.get("completion", "")


def agent_step(session_id: str, user_input: str) -> str:
    resp = _post("/v1/agent", {"session_id": session_id, "input": user_input})
    return resp.get("output", "")


def list_tools() -> dict:
    with urllib.request.urlopen(BASE_URL + "/v1/tools") as resp:
        return json.loads(resp.read().decode("utf-8"))


if __name__ == "__main__":
    print("Completion:", complete("Say hello"))
    print("Tools:", list_tools())

