from __future__ import annotations

from pathlib import Path

_PACKAGE_ROOT = Path(__file__).resolve().parent


def samples_xml_path() -> Path:
    return _PACKAGE_ROOT / "samples.xml"


def samples_directory() -> Path:
    return _PACKAGE_ROOT / "samples"


def get_samples_xml_path() -> str:
    return str(samples_xml_path())


def get_samples_directory() -> str:
    return str(samples_directory())


__all__ = [
    "samples_xml_path",
    "samples_directory",
    "get_samples_xml_path",
    "get_samples_directory",
]
