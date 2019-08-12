from setuptools import setup, find_packages

setup(
    name="Geeky",
    version="0.1",
    packages=find_packages(),
    install_requires=["scrapy", "pil-compat", "pytesseract", "PyMySQL"],
)
