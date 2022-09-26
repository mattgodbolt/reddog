// error fix is set to zero now for spe-e-e-d
#define INSIDE_POLY \
			if (vcC > 0.3f) \
			{ \
				if (vcvs_pp.x * vcve_vs.y - vcvs_pp.y * vcve_vs.x < 0) \
					break; \
			} \
			else if (vcC < -0.3f) \
			{ \
				if (vcvs_pp.x * vcve_vs.y - vcvs_pp.y * vcve_vs.x > 0) \
					break; \
			} \
			else if (vcA < -0.3f) \
			{ \
				if (vcvs_pp.y * vcve_vs.z - vcvs_pp.z * vcve_vs.y > 0) \
					break; \
			} \
			else if (vcA > 0.3f) \
			{ \
				if (vcvs_pp.y * vcve_vs.z - vcvs_pp.z * vcve_vs.y < 0) \
					break; \
			} \
			else if (vcB < -0.3f) \
			{ \
				if (vcvs_pp.z * vcve_vs.x - vcvs_pp.x * vcve_vs.z > 0) \
					break; \
			} \
			else if (vcB > 0.3f) \
			{ \
				if (vcvs_pp.z * vcve_vs.x - vcvs_pp.x * vcve_vs.z < 0) \
					break; \
			} \


#define INSIDE_POLY2 \
			if (C > 0.3f) \
			{ \
				if (vs_pp.x * ve_vs.y - vs_pp.y * ve_vs.x < 0) \
					break; \
			} \
			else if (C < -0.3f) \
			{ \
				if (vs_pp.x * ve_vs.y - vs_pp.y * ve_vs.x > 0) \
					break; \
			} \
			else if (A < -0.3f) \
			{ \
				if (vs_pp.y * ve_vs.z - vs_pp.z * ve_vs.y > 0) \
					break; \
			} \
			else if (A > 0.3f) \
			{ \
				if (vs_pp.y * ve_vs.z - vs_pp.z * ve_vs.y < 0) \
					break; \
			} \
			else if (B < -0.3f) \
			{ \
				if (vs_pp.z * ve_vs.x - vs_pp.x * ve_vs.z > 0) \
					break; \
			} \
			else if (B > 0.3f) \
			{ \
				if (vs_pp.z * ve_vs.x - vs_pp.x * ve_vs.z < 0) \
					break; \
			} \

