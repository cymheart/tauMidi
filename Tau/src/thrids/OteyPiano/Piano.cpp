/* coded by Clayton Otey
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "piano.h"
#include <string.h>


namespace oteypiano
{

	float TUNE[3] = { 0, 1, -1 };
	float F_NOTE_31 = 6.875 * pow(2.0, (31.0 + 3) / 12.0);
	float F_NOTE_41 = 6.875 * pow(2.0, (41.0 + 3) / 12.0);

	float linearmap(float s, float e, float ds, float de, float v) {
		return ((de - ds) * (v - s) / (e - s)) + ds;
	}

	float sigmoidal(float midi, float minV, float maxV, float ampL, float ampR) {
		//float ampL = amp * center;
		//float ampR = amp * (1 - center);
		//Print("sigmoidal %g %g %g %g\n",minV,maxV, ampL, ampR);
		float i = linearmap(21, 108, ampL, ampR, midi);

		float minV2 = 1 / (1 + exp(ampR));
		float maxV2 = 1 / (1 + exp(ampL));

		float escale = (maxV - minV) / (maxV2 - minV2);

		float offset = maxV - maxV2 * escale;

		float val = offset + escale / (1 + exp(i));
		return val;
	}


	const int DWGReverb::lengths[8] = { 37,87,181,271,359,593,688,721 };
	//Householder Feedback Matrix
	void DWGReverb::setcoeffs(float c1, float c3, float a, float mix, float Fs) {
		//lengths = {{37,87,181,271,359,592,687,721}};
		float aa[8] = { a,1 + a,a,a,a,a,a,a };
		this->mix = mix;

		for (int k = 0; k < 8; k++) {
			o[k] = 0;
			b[k] = 1;
			c[k] = k < 8 ? ((k % 2 == 0) ? 1.0 / 8.0 : -1.0 / 8.0) : 0.0;
			decay[k].setcoeffs(Fs / lengths[k], c1, c3);
		}
		for (int j = 0; j < 8; j++)
			for (int k = 0; k < 8; k++)
				A[j][k] = aa[(8 + (k - j)) % 8];
	}

	float DWGReverb::tick(float in)
	{
		float i[8];

		for (int j = 0; j < 8; j++) {
			i[j] = b[j] * in;
			for (int k = 0; k < 8; k++) {
				i[j] += A[j][k] * o[k];
			}
		}

		float out = 0;
		for (int j = 0; j < 8; j++) {
			delay[j].push(i[j]);
			o[j] = decay[j].filter(delay[j].delay(lengths[j]));
			kill_denormals(o[j]);
			//o[j] = zapgremlins(o[j]);
			out += c[j] * o[j] * .5;
		}

		return mix * out + (1.0 - mix) * in;
	}


	void PianoSoundBoard::Init(PianoSoundboardParams& param)
	{
		this->param.c1 = param.c1;
		this->param.c3 = param.c3;
		this->param.eq1 = param.eq1;
		this->param.eq2 = param.eq2;
		this->param.eq3 = param.eq3;

		shaping1.setcoeffs(param.eq1, 44100, 10, Biquad::notch);
		shaping2.setcoeffs(param.eq2, 44100, 1.0, Biquad::high);
		shaping3.setcoeffs(param.eq3, 44100, 1.0, Biquad::low);
		reverb.setcoeffs(param.c1, param.c3, -0.25f, 1, 44100);
	}

	void PianoSoundBoard::Go(float* in, float* out, int count)
	{
		float signal;
		for (int i = 0; i < count; i++) {
			signal = reverb.tick(in[i]);
			signal += shaping1.filter(signal);
			signal = shaping2.filter(signal);
			signal += shaping3.filter(signal);
			out[i] = signal;
		}
	}


	/////////////////////////////////////////////////////////////////////
	PianoKey::PianoKey()
	{

	}


	PianoKey :: ~PianoKey() {
		Clear();
	}


	void PianoKey::Clear()
	{
		for (int k = 0; k < nstrings; k++) {
			if (string[k] != nullptr) {
				delete string[k];
				string[k] = nullptr;
			}
		}

		if (hammer != nullptr) {
			delete hammer;
			hammer = nullptr;
		}
	}

	void PianoKey::Init(PianoKeyParams& param)
	{
		//this->amp = amp * 100.0;
		float f0 = 27.5;
		float midinote = 12 * log(param.f / f0) / log(2) + 21;
		//float L = .04 + 1.4/(1+exp(-3.4+1.4*log(f/f0)));
		float L = sigmoidal(midinote, param.minL, param.maxL, param.ampLl, param.ampLr);
		//float r = .002*pow(1+.6*log(f/f0),-1.4);
		float r = sigmoidal(midinote, param.minr, param.maxr, param.amprl, param.amprr) * 0.001;
		float rho = 7850.0;
		rho *= param.mult_density_string;
		float logff0 = log(param.f / f0) / log(4192 / f0);
		logff0 = (0 > logff0) ? 0 : logff0;
		float m = .06 - .058 * pow(logff0, .1);
		m *= param.mult_mass_hammer;
		float alpha = 0.1e-4 * logff0;
		alpha *= param.mult_hysteresis_hammer;
		float p = 2.0 + 1.0 * logff0;
		p *= param.mult_stiffness_exponent_hammer;
		float K = 40.0 / pow(.7e-3, p);
		K *= param.mult_force_hammer;
		Zb = 4000.0 * param.mult_impedance_bridge;
		Zh = 1.0 * param.mult_impedance_hammer;
		float rcore = (r < .0006) ? r : .0006;
		rcore *= param.mult_radius_core_string;
		float E = 200e9;
		E *= param.mult_modulus_string;
		float pos = param.position_hammer;

		Fs = param.Fs;
		v0 = param.velocity;
		f = param.f;

		if (f < F_NOTE_31)
			nstrings = 1;
		else if (f < F_NOTE_41)
			nstrings = 2;
		else
			nstrings = 3;

		float c1 = 0.25;
		float c3 = 5.85;
		c3 *= param.mult_loss_filter;
		float rhoL = PI * r * r * rho;
		float T = (2 * L * f) * (2 * L * f) * rhoL;
		Z = sqrt(T * rhoL);
		float B = (PI * PI * PI) * E * rcore * rcore * rcore * rcore / (4.0 * L * L * T);

		for (int k = 0; k < nstrings; k++) {
			string[k] = new dwgs(f * (1.0 + TUNE[k] * param.detune), Fs, pos, c1, c3, B, Z, Zb + (nstrings - 1) * Z, Zh);
		}

		switch (param.hammer_type) {
		case 1:
			hammer = new StulovHammer(f, Fs, m, K, p, Z, alpha, v0);
			break;
		case 2:
			hammer = new BanksHammer(f, Fs, m, K, p, Z, alpha, v0);
			break;
		default:
			hammer = new StulovHammer(f, Fs, m, K, p, Z, alpha, v0);
		}
	}


	void PianoKey::Trigger(float v)
	{
		hammer->trigger(v * v0);
	}


	long PianoKey::Go(float* out, int samples)
	{
		long n = 0;
		float Zx2 = 2 * Z;
		float facZ = Zx2 / (Z * nstrings + Zb);

		for (int i = 0; i < samples; i++) {

			n++;
			float vstring = 0.0;
			for (int k = 0; k < nstrings; k++) {
				vstring += string[k]->input_velocity();
			}

			float hload = hammer->load(vstring / nstrings);
			float load = 0;

			for (int k = 0; k < nstrings; k++) {
				load += string[k]->go_hammer(hload / (Zx2));
			}
			load *= facZ;
			float output = 0.0;
			for (int k = 0; k < nstrings; k++) {
				output += string[k]->go_soundboard(load);
			}


			out[i] = output * 100;
		}
		return n;
	}
}



