
#include <math.h>

#define PI_F	3.14159265f

void hamming(float *out, float *in, short len) {
	short i;
	
	for (i = 0; i < len; i++) {
		out[i] = in[i] * (0.54f - 0.46f * cos(2.0f * PI_F * i / (len - 1)));
	}
}



void yule(float *out, short n, float *in, short len) {
	// Solves for n autocorrelation coefficients
	short i, j;
	
	for (i = 0; i < n; i++) {
		out[i] = 0;
		for (j = 0; j < len - i; j++) {
			out[i] += in[j] * in[j + i];
		}
		out[i] /= len;
	}
}




void levinson(float *out, short p, float *in, short len) {
	// Uses p + 1 autocorrelation coefficients to create p lpcs
	short m, i;
	float P[p], a[p][p], acc[p + 1];
	
	yule(acc, p + 1, in, len);

	a[0][0] = -acc[1] / acc[0];
	P[0] = acc[0] * (1.0f - (a[0][0] * a[0][0]));
	
	for (m = 1; m < p; m++) {
		a[m][m] = 0.0f;
		for (i = 0; i < m; i++) {
			a[m][m] += a[m - 1][i] * acc[m - i];
		}
		a[m][m] = -(acc[m + 1] + a[m][m]) / P[m - 1];
		
		for (i = 0; i < m; i++) {
			a[m][i] = a[m - 1][i] + (a[m][m] * a[m - 1][m - i - 1]);
		}
		
		P[m] = P[m - 1] * (1.0f - (a[m][m] * a[m][m]));
	}
	
	for (i = 0; i < p; i++) {
		out[i] = a[p - 1][i];
	}
}
	
	
	
void cepstral(float *out, short p, float *in, short len) {
	// Calculates p quefrency coefficients for a signal
	short k, n;
	float apk[p];
	
	levinson(apk, p, in, len);
	
	for (k = 0; k < p; k++) {
		out[k] = 0.0f;
		for (n = 0; n < k; n++) {
			out[k] += (float)(k - n) * out[k - n - 1] * apk[n];
		}
		out[k] = -apk[k] - (out[k] / (float)(k + 1));
	}
	for (k = 0; k < p; k++) {
		out[k] = (float)(k + 1) * out[k];
	}
}
	


float difference(float (*testing)[FEATURES_PER_FRAME], float (*trained)[FEATURES_PER_FRAME], short numFrames) {
	float frameDist, minFrameDist, avgDist;
	short i, j, k;
	
	avgDist = 0.0f;
	for (i = 0; i < numFrames; i++) {
		if (trained[i][0] == SILENT_FRAME) { continue; }		// Skip silent frame
		minFrameDist = MASSIVE_NUMBER;
		for (j = 0; j < numFrames; j++) {
			if (testing[j][0] == SILENT_FRAME) { continue; }	// Skip silent frame
			frameDist = 0.0f;
			for (k = 0; k < FEATURES_PER_FRAME; k++) {
				frameDist += (trained[i][k] - testing[j][k]) * (trained[i][k] - testing[j][k]);
			}
			if (frameDist < minFrameDist) {
				minFrameDist = frameDist;
			}
		}
		avgDist += minFrameDist;
	}
	return avgDist / numFrames;
}
