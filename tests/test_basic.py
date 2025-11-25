import fastwfc as m
from fastwfc_resources import samples_directory, samples_xml_path


def test_main():
    assert m.__version__ == "0.0.1"
    assert m.add(1, 2) == 3
    assert m.subtract(1, 2) == -1


def test_packaged_resources_available():
    xml_path = samples_xml_path()
    samples_dir = samples_directory()
    assert xml_path.exists()
    assert samples_dir.exists()
    assert xml_path.parent == samples_dir.parent


def test_xlandwfc_default_config_initializes():
    model = m.XLandWFC()
    wave = model.build_a_open_area_wave()
    data = wave.get_data()
    assert len(data) > 0
