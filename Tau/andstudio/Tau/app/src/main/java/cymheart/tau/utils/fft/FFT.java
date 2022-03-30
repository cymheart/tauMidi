package cymheart.tau.utils.fft;

public class FFT {

    public double[] DoFFT(double[] data, int isInverse)
    {
        return dofft(data, isInverse);
    }


    private native double[] dofft(double[] data, int is_inverse);

}
