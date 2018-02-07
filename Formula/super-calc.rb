class SuperCalc < Formula
  desc "SuperCalc is a mathematical expression parser & evaluator, plus more"
  homepage "https://github.com/ctrezevant/SuperCalc"
  url "https://github.com/ctrezevant/SuperCalc.git"
  version "1.0.0"
 
  # Needed to override Brew's Superenv
  env :std

  def install
    system "./autogen.sh"
    system "./configure"
    system "make"
    bin.install "sc"
  end

  test do
    system "#{bin}/sc"
  end
end
