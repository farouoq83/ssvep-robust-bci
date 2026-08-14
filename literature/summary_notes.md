# 15-Paper Summary — SSVEP-BCI Literature Review
### B1 English version | 6-field template | For: "Extraction of Robust Commands from SSVEP Signals for BCI"

**Note on this rewrite:** Papers 1 and 2 are new (they had no summary before). Papers 11–14 are also new. Papers 3, 4, 5, 5.2, 6, 7, 8, 9, and 10 are rewritten from the old summary, in simpler B1 English. All facts come from reading the actual PDF files, not from memory. Numbers marked with ⚠️ still need a personal check against the source figure/table before you cite them in your report.

---

## 1. Steady-State Visually Evoked Potentials: Focus on Essential Paradigms and Future Perspectives
*Vialatte, Maurice, Dauwels & Cichocki (2010), Progress in Neurobiology*

**1. Problem Addressed:** This is a big review paper, not a new experiment. It asks: what do we know about SSVEPs after 40 years of research, in neuroscience, clinical medicine, and brain-computer interfaces (BCIs)?

**2. Method/Algorithm:** The authors read and organized many previous studies. They explain what SSVEPs are, where they come from in the brain, and how they are used in BCI systems.

**3. Key Result:** SSVEP-based BCIs reach information transfer rates (ITR, a speed-and-accuracy score) of 100 bits/min or more. Non-SSVEP BCIs usually only reach 10–25 bits/min. In a table of 13 SSVEP-BCI studies, accuracy ranged from 64% to 96.5%, with average ITR between 2.3 and 58 bits/min (best case 70 bits/min). Over 90% of users can operate an SSVEP-BCI without training.

**4. Robustness Claim:** SSVEP signals stay stable in frequency and phase (timing) over long periods. They are less disturbed by eye blinks and muscle noise than other brain signals used in BCIs.

**5. Stated Limitation:** The authors admit that basic SSVEP properties — like the exact frequency range and long-term stability — are still not fully understood. Different studies use different setups, so comparing results across papers is hard.

**6. Connection to My Argument:** This paper is the foundation. It gives the taxonomy (dependent vs. independent BCI, feature type) and the 5 design steps (paradigm, stimuli, channels, noise reduction, detection) that every later paper in my reading list builds on. It also gives me baseline ITR numbers to compare my own SSVEP results against.

---

## 2. Brain-Computer Interfaces: A Review
*Nicolas-Alonso & Gomez-Gil (2012), Sensors*

**1. Problem Addressed:** This is another big review paper. It covers the whole BCI pipeline — signal acquisition, preprocessing, feature extraction, classification, and control — across many BCI types (not only SSVEP).

**2. Method/Algorithm:** Literature review of neuroimaging methods (EEG, fMRI, MEG, NIRS), control signal types (VEP/SSVEP, slow cortical potentials, P300, sensorimotor rhythms), and signal-processing algorithms.

**3. Key Result:** A summary table compares control signals: VEP/SSVEP reaches 60–100 bits/min ITR with no training needed; P300 reaches 20–25 bits/min; sensorimotor rhythms reach 3–35 bits/min but need training. The paper also splits SSVEP-BCIs into three types by stimulus coding: time-modulated (t-VEP, <30 bits/min, no training), frequency-modulated (f-VEP, 30–60 bits/min, no training), and code-modulated (c-VEP, >100 bits/min, **training required**).

**4. Robustness Claim:** SSVEP/VEP signals are called "exogenous" — they are driven by an outside stimulus, not by the user's own effort. This makes them easier to detect reliably than "endogenous" signals like sensorimotor rhythms, which depend on the user's mental state.

**5. Stated Limitation:** The authors say (in their own conclusion) that current BCIs still have low information transfer rates for real-world use, that invasive methods carry health risks, and that most BCI systems are still uncomfortable and not ready for daily home use.

**6. Connection to My Argument:** The t-VEP / f-VEP / c-VEP taxonomy matches my own hardware design exactly (SSVEP mode + cVEP mode on the same LED panel). It also confirms, from the literature, that c-VEP needs training data — which matches what I found myself: training-free cVEP decoding failed on my data (1/8), while calibrated SSVEP (FBCCA) worked well (8/8).

---

## 3. Frequency Recognition Based on Canonical Correlation Analysis for SSVEP-Based BCIs
*Lin & Zhang (2007)*

**1. Problem Addressed:** The authors wanted to find the best EEG channels (locations) to record SSVEP signals, and to improve target recognition using Canonical Correlation Analysis (CCA) instead of an older method called PSDA (Power Spectral Density Analysis).

**2. Method/Algorithm:** They used CCA — a statistic that measures how well two sets of signals (EEG data and reference sine/cosine waves) are correlated — together with a small set of "bipolar" electrode pairs chosen for high signal-to-noise ratio (SNR).

**3. Key Result:** Eight channels gave the best accuracy. CCA reached higher accuracy than PSDA at different signal lengths, for almost every subject (one subject, called NTC, was an exception).

**4. Robustness Claim:** A smaller standard deviation (STD) means a more stable, more robust method. The paper shows CCA is more robust than PSDA. Using more channels also increases robustness against noise.

**5. Stated Limitation:** The method only uses the single largest CCA coefficient to make its decision — it throws away extra information. The authors also note that the brain's visual pathway is not perfectly linear, but CCA assumes linear relationships.

**6. Connection to My Argument:** This paper tells me which channels and how many channels to prioritize in my own signal-processing pipeline, as a first pre-processing step before applying CCA/FBCCA.

---

## 4. Multivariate Synchronization Index for Frequency Recognition of SSVEP-Based Brain-Computer Interface
*(2014)*

**1. Problem Addressed:** Older multichannel methods like CCA needed calibration and manual tuning (choosing channels, choosing how much data) for every user. The authors wanted a method that works well without this tuning, especially with short data and few electrodes.

**2. Method/Algorithm:** Multivariate Synchronization Index (MSI). It measures how "in sync" the real EEG signal is with reference sine/cosine waves at each candidate frequency, using a math tool called the S-estimator (a score from 0 to 1). The frequency with the highest S score is chosen as the answer.

**3. Key Result:** In simulations with very noisy signals (SNR from −7 dB to −20 dB), MSI beat CCA and another method called MEC, especially with short data (1 s) and few channels (4). In a real offline EEG test with 11 subjects, MSI again beat CCA/MEC in almost every condition. In an online robot-control test (10 subjects), average accuracy was 86% (±13.08%) and average ITR was 19.43 bits/min (±8.16).

**4. Robustness Claim:** MSI works better than CCA/MEC specifically with short data and few channels — the two conditions that limit real-world usability most. It also avoids relying only on a linear combination of channels.

**5. Stated Limitation:** The online ITR (19.43 bits/min) is lower than other systems, mainly because only 4 targets were used. One subject scored only 60% while two others scored 100% — showing large individual differences remain, even with MSI.

**6. Connection to My Argument:** This gives me a third detection method (with CCA and TRCA) for the same goal: better target detection with less data and fewer electrodes. It shows the field's progress over time: CCA → MSI → TRCA, all aiming at shorter, more robust detection.

---

## 5. Filter Bank Canonical Correlation Analysis for Implementing a High-Speed SSVEP-Based Brain-Computer Interface
*Journal of Neural Engineering*

**1. Problem Addressed:** Standard CCA does not use harmonics well. Harmonics are the extra brain responses at 2×, 3×, 4× the flicker frequency, which also carry useful information. An earlier attempt to add harmonics to CCA's reference signal did not help.

**2. Method/Algorithm:** Filter Bank CCA (FBCCA). It splits the raw EEG into several frequency sub-bands (like a graphic equalizer), runs CCA separately on each sub-band, then combines all sub-band scores into one final score, giving more weight to lower-frequency sub-bands. Three sub-band designs were tested; the best one (M3) makes each sub-band start at a harmonic but always extend up to 88 Hz.

**3. Key Result:** Offline (12 subjects): standard CCA reached 76.80% accuracy (113.85 bits/min); FBCCA-M3 reached 89.47% (145.52 bits/min) — the best of the three designs. Online (10 subjects, M3): 91.95±7.22% accuracy, 151.18±20.34 bits/min ITR, at a speed of 33.3 characters/minute.

**4. Robustness Claim:** Even though harmonic amplitude drops fast at higher frequencies, the signal-to-noise ratio (SNR) drops much more slowly, because background brain noise also weakens. This is why using harmonics still helps, and it works without any user-specific calibration.

**5. Stated Limitation (authors' own words):** This was a cue-guided task, not free/independent spelling. "BCI illiteracy" (some users respond poorly to SSVEP) was not tested on a large population. Low frequencies (8–15.8 Hz) may cause visual fatigue. FBCCA is slower to compute than plain CCA (about 40 ms per detection — still fast enough for real time). Methods that use personal calibration data can beat FBCCA in accuracy (97% vs. 92%), but only by requiring a calibration session that FBCCA avoids.

**6. Connection to My Argument:** FBCCA is the exact method I used in my own pipeline (3 sub-bands, weighted scoring), and it reached 8/8 accuracy on my own recordings — matching this paper's claim that harmonic exploitation, without calibration, gives strong results.

---

## 5.2. High-Speed Spelling with a Noninvasive Brain-Computer Interface
*Chen et al. (2015), PNAS*

**1. Problem Addressed:** Existing SSVEP spellers, including FBCCA, still needed fairly long data windows for high accuracy. The authors asked: can we push SSVEP-BCI speed further by using the timing (phase) of the SSVEP signal, not just its frequency?

**2. Method/Algorithm:** Joint Frequency-Phase Modulation (JFPM). Each of the 40 targets gets both a unique frequency and a unique phase offset (a small timing shift). This "double-tagging" makes targets distinguishable with a much shorter data window (0.5 s instead of 5 s). The best settings were found by grid search (systematically testing many combinations of stimulus duration and phase interval).

**3. Key Result:** Best offline result: 0.5 s stimulation, 0.35π phase interval → 88.92% accuracy, 4.32 bits/second (bps). Online cued-spelling: 91.04±6.73% accuracy, 4.45±0.58 bps (about 267 bits/min after converting units — bps and bits/min are different!). Online free-spelling (no cues): 4.50±1.03 bps.

**4. Robustness Claim:** This speed gain depends on the brain's visual response delay (about 140 ms) being very stable from trial to trial. The authors tested this and found only about 1.7 ms of variation (⚠️ verify this number in the paper's supplementary Figure S1 before citing it — I have not personally viewed the supplementary file).

**5. Stated Limitation (authors' own words):** Subjects were experienced users familiar with the target layout; naive users were not tested. Three subjects needed a longer gaze-shifting time. Long-term stability of visual latency over many sessions is explicitly called "unknown." The fixed, equally-spaced frequency/phase design may not be optimal.

**6. Connection to My Argument:** This paper adds a third robustness "lever" — stable phase timing — next to spatial filtering (TRCA) and harmonic use (FBCCA), supporting my argument that "robustness" in SSVEP-BCI is not one single thing but several independent mechanisms.

---

## 6. Enhancing Detection of SSVEPs for a High-Speed Brain Speller Using Task-Related Component Analysis
*(TRCA paper)*

**1. Problem Addressed:** How to identify, quickly and accurately, which of 40 flickering targets a user is looking at. Existing methods like Extended CCA were not fast or accurate enough for high-speed spelling.

**2. Method/Algorithm:** Task-Related Component Analysis (TRCA) — a spatial filter that finds channel weights making the signal as consistent as possible across repeated trials of the same target. Extended into Ensemble TRCA (eTRCA), which combines filters from all 40 targets, plus filter-bank analysis to weight useful frequency sub-bands more.

**3. Key Result:** Ensemble TRCA reached its best accuracy with only 300 ms of data (vs. 400 ms for TRCA alone or Extended CCA). Online cue-guided speller: mean accuracy 89.83±6.07%, mean ITR 325.33±38.17 bits/min — described by the authors as the highest ITR reported in EEG-BCI research at that time. Online free-spelling (no cues): mean ITR 198.67±50.48 bits/min.

**4. Robustness Claim:** TRCA's spatial filtering removes background EEG noise and significantly increases the separation between target and non-target signals (statistically significant, p<0.05 to p<0.01). This lets the system stay accurate even with short data windows.

**5. Stated Limitation:** The high spelling speed (0.8 s/character) needed trained, experienced users. Naive users would likely need 1–2 s per gaze shift. A fixed stimulus duration was used; the authors admit that adaptively deciding how long to look ("dynamic stopping") was not tested but could improve speed further.

**6. Connection to My Argument:** This shows a clear causal chain: spatial filtering → less noise → better signal separation → higher accuracy with shorter data → more robust command extraction. The authors' own untested gap (no dynamic stopping) is a possible future direction to propose to Prof. Seyedarabi.

---

## 7. To Train or Not to Train? A Survey on Training of Feature Extraction Methods for SSVEP-Based BCIs
*Zerafa et al. (2018)*

**1. Problem Addressed:** SSVEP-BCIs need less calibration (a training phase where the system learns your personal brain patterns) than other BCI types, but some calibration is often still needed for good accuracy. The paper asks: how much calibration is really needed, and what do you gain or lose by skipping it?

**2. Method/Algorithm:** This is a survey, not a new experiment. The authors reviewed peer-reviewed papers on SSVEP feature extraction and sorted every method into three categories: **training-free** (works immediately, e.g., CCA), **subject-specific** (needs calibration data from that one user), and **subject-independent** (needs calibration data from many other users, but not from the end-user).

**3. Key Result:** Methods that use training (subject-specific or subject-independent) generally beat the popular training-free method CCA. ⚠️ Reported gaps (pulled directly from the paper's Figure 4/5 — verify yourself before citing): training-free methods showed 7%–41% lower accuracy and 17%–65% lower ITR than subject-specific methods; and 19%–46% lower accuracy and 28%–63% lower ITR than subject-independent methods.

**4. Robustness Claim:** The authors are careful here — these gaps come from comparing different studies, not one controlled experiment. They explicitly warn about "large variations in the performances of the same method across different studies," so these numbers are only a rough trend, not a hard robustness metric.

**5. Stated Limitation:** Very few studies directly compare subject-specific vs. training-free methods — and even fewer compare subject-specific vs. subject-independent methods at all. The authors call this a "glaring gap in the literature."

**6. Connection to My Argument:** This paper is the structural backbone of my whole literature review. Its three-way taxonomy organizes all the other papers. And its stated gap — no direct, controlled comparison of subject-specific vs. subject-independent training — is a real, citable, unaddressed opportunity for the novel contribution I could propose to Prof. Seyedarabi.

---

## 8. EEGNet: A Compact Convolutional Neural Network for EEG-Based Brain-Computer Interfaces

**1. Problem Addressed:** Can one small, compact CNN (Convolutional Neural Network) architecture work well across several very different BCI types (event-related potentials and rhythm-based signals), even with limited training data?

**2. Method/Algorithm:** EEGNet — a CNN using depthwise and separable convolutions (efficient building blocks that reduce the number of parameters). Compared against bigger CNNs (DeepConvNet, ShallowConvNet) and traditional pipelines (xDAWN+Riemannian Geometry, FBCSP).

**3. Key Result:** Within-subject, EEGNet matched or outperformed the bigger CNNs on several tasks (P300, ERN, MRCP), and matched FBCSP on a sensorimotor-rhythm task. Cross-subject results were more mixed: for one specific task (ERN), the traditional xDAWN+Riemannian method clearly beat all CNNs, including EEGNet.

**4. Robustness Claim:** The authors argue EEGNet's robustness comes from being interpretable: using three visualization techniques, they showed EEGNet's learned spatial filters resemble known brain-signal patterns (e.g., similar to FBCSP's 8–12 Hz filters), suggesting good performance was not just from noise or artifacts.

**5. Stated Limitation:** The cross-subject failure on the ERN task is an honest, self-reported failure case for deep learning. The authors also note that DeepConvNet needed extra data augmentation in an earlier study, while EEGNet did not — but this is an indirect observation, not something they tested directly in this paper.

**6. Connection to My Argument:** I can present EEGNet's depthwise convolution as a "learned" version of TRCA/CCA's hand-designed spatial filter — both solve the same problem (combine channels to isolate the useful signal), just using different optimization goals. This supports my argument that deep learning's promise for BCI robustness is about generalizing without needing to hand-tune the method for every new paradigm.

---

## 9. An Analysis of Traditional Methods and Deep Learning Methods in SSVEP-Based BCI: A Survey

**1. Problem Addressed:** No single, unified comparison exists between traditional classifiers (KNN, MLP, SVM) and deep learning classifiers for SSVEP-BCI.

**2. Method/Algorithm:** A literature survey of 66 papers (2015–2023), sorted by paradigm, decoding method, and classifier type.

**3. Key Result:** CNNs generally outperform traditional classifiers for SSVEP specifically, because SSVEP information is concentrated in a specific brain region (occipital area), which suits CNNs' spatial pattern-finding. However, SVM (a traditional method) still wins when the dataset is small.

**4. Robustness Claim:** Combinations of filter-bank preprocessing with deep learning (like FB-CCNN) reach the highest reported accuracies (94.85±6.18%) among the zero-calibration (training-free) methods reviewed.

**5. Stated Limitation (authors' own words):** Deep learning models need large labeled datasets and long training time, which is impractical for real-time BCI without further optimization. Short-time-window, high-ITR algorithms that also avoid calibration remain rare across the whole field.

**6. Connection to My Argument:** This paper gives me a field-level organizing framework (three decoding categories, deep-learning-vs-traditional trade-offs) for my whole literature review. It also confirms, from a large 66-paper survey, that my assigned topic (short-window, robust command extraction) is a real, field-wide, unsolved gap — strong support for the "testable research angle" I want to propose to Prof. Seyedarabi.

---

## 10. SSVEP-Based Brain-Computer Interfaces Are Vulnerable to Square Wave Attacks

**1. Problem Addressed:** Earlier adversarial attacks on EEG-BCIs needed to see the whole EEG trial first (breaking real-time causality) or needed complex, channel-varying perturbations. Can a simpler, real-time attack still succeed against SSVEP-BCIs?

**2. Method/Algorithm:** Injecting a square-wave signal, targeted at CCA and FBCCA classifiers (the two training-free methods from my reading list). Tested on the Benchmark (35 subjects, 40 targets) and BETA (70 subjects, 40 targets) public SSVEP datasets.

**3. Key Result:** At only 30% of the signal's normal standard deviation in amplitude, attacking 4 occipital channels (PO3, POz, PO4, Oz) together reached a near-100% attack success rate on Benchmark and about 90% on BETA. Even a single channel (PO3) came close. The attack worked regardless of its timing (phase) or the trial's length.

**4. Robustness Claim:** From the attacker's point of view, the attack itself is "robust": it survives standard bandpass filtering, works no matter when it starts, and works no matter how long the trial is. (Note the dual meaning of "robustness" here — this is robustness of the attack, not of the BCI.)

**5. Stated Limitation:** The attack only defeats training-free models (CCA, FBCCA). Training-based models (extended CCA, multi-stimulus TRCA, TRCA variants) needed much larger, more detectable perturbations to attack successfully. The attack also needs to start from the very beginning of the trial, a harder real-time requirement than the headline numbers suggest.

**6. Connection to My Argument:** This gives me a direct, citable comparison: TRCA-family (training-based) methods are inherently harder to attack than CCA/FBCCA (training-free) methods — a real robustness advantage for training-based approaches, feeding directly into my "adversarial robustness" dimension.

---

## 11. Deep Learning Decoding of Steady-State Visual Evoked Potential (SSVEP) for Real-Time Mobile Brain-Computer Interfaces: A Narrative Review
*Zhang & Tao (2026), Brain Sciences*

**1. Problem Addressed:** No review yet focuses on deep-learning SSVEP decoding specifically for real-time mobile BCIs, using the standard Tsinghua Benchmark dataset (studies published since 2023).

**2. Method/Algorithm:** A narrative literature review, organized around three engineering-focused technical directions: lightweight model architectures, ultra-short time-window decoding, and cross-subject transfer learning.

**3. Key Result:** Some lightweight models reach very high performance with very few parameters: FBCNN-TKS reaches 251.54 bits/min ITR using only 0.4 s of data; GZSL-Lite reaches 200.28 bits/min with just 4,270 trainable parameters. For cross-subject generalization, the Adaptive Euclidean Alignment (AEA) method uses only 0.013% of the trainable weights compared to a non-transfer model.

**4. Robustness Claim:** Transfer learning and domain-alignment methods partially fix the accuracy drop seen when a deep learning model meets a brand-new subject. But the authors call this "equilibrium" fragile: very lightweight models may lack long-term stability, while high-performance models sacrifice computing efficiency.

**5. Stated Limitation (authors' own words):** Using only the Tsinghua Benchmark dataset may exclude useful algorithms tested elsewhere. No unified evaluation standard exists yet across studies. The risk of overfitting on this relatively small benchmark, and how models behave under real failure conditions (user fatigue, strong ambient light, poor electrode contact), remain unexplored.

**6. Connection to My Argument:** This very recent (2026) review directly names cross-subject/low-calibration robustness as the field's current "active bottleneck" — matching the central three-dimension structure of my own report's argument (within-trial vs. cross-subject vs. adversarial robustness).

---

## 12. Facilitating Applications of SSVEP-BCI by Effective Cross-Subject Knowledge Transfer
*Li et al. (2024), Expert Systems With Applications*

**1. Problem Addressed:** Existing unsupervised cross-subject transfer learning methods focus only on "what to transfer," not "how to transfer effectively" — this can cause poor results, or even "negative transfer," for a brand-new user with zero calibration data.

**2. Method/Algorithm:** SUTL (Subject-transferability Unsupervised Transfer Learning). It combines two new ideas: Subject Transferability Estimation (STE), which picks the most useful "source" subjects from a pool, and multi-domain alignment (using Euclidean Alignment), which makes different subjects' signal distributions more similar to each other.

**3. Key Result:** On the Benchmark dataset, SUTL reached 75.53±12.72% accuracy versus 58.90±14.51% for FBCCA, 66.30±15.03% for tt-CCA, and 65.00±13.62% for CSSFT. On the BETA dataset, SUTL reached 68.18±14.82% versus roughly 54–58% for the other methods. Best ITR was 265.31±95.08 bits/min at a 0.8 s window.

**4. Robustness Claim:** SUTL still performs well even without many source subjects, and stays computationally practical (about 183 ms per trial with 20 source subjects, versus 14–24 ms for the simpler baseline methods).

**5. Stated Limitation (authors' own words):** The domain alignment step only reduces, but does not fully remove, the differences between subjects' signal distributions. Computation time grows as more source subjects are used. SUTL still needs a pool of pre-recorded source subjects — it is not truly zero-data for a completely new lab setup.

**6. Connection to My Argument:** This is a concrete, controlled example of subject-independent training that could serve as a comparison point for the head-to-head subject-specific vs. subject-independent study that Zerafa et al. (paper 7) identified as missing from the literature.

---

## 13. Enhancing the Performance of SSVEP-Based BCIs by Combining Task-Related Component Analysis and Deep Neural Network
*Wei, Li, Wang & Gao (2025), Scientific Reports*

**1. Problem Addressed:** No efficient, established way exists to combine a traditional machine-learning method (TRCA) and a deep-learning method to get the benefits of both for SSVEP decoding.

**2. Method/Algorithm:** eTRCA+sbCNN — train an ensemble TRCA model and a sub-band CNN (sbCNN) separately, then add their classification score vectors together and pick the frequency with the highest combined score.

**3. Key Result:** On the Benchmark dataset at 1.0 s, eTRCA+sbCNN reached 96.19% accuracy (197.48 bits/min ITR), higher than TRCA alone (88.43%, 174.00 bits/min) and other strong models like Conv-CA (92.99%) and bi-SiamCA (94.07%). On the BETA dataset, it reached 84.88% accuracy (159.96 bits/min) versus TRCA's 72.89%.

**4. Robustness Claim:** The combination works because the two models are complementary: sbCNN is fully data-driven and generalizes well across subjects, while eTRCA is built from known SSVEP physiology and fits the paradigm precisely. However, the authors note the combination only helps when the two models' individual performance is not too different — at a very short 0.2 s window, sbCNN alone already dominates, so combining them adds little.

**5. Stated Limitation (authors' own words):** Total training time was too long (about 2.4 hours) for practical new-user calibration, because sbCNN needs a two-stage training process. Only offline analysis was done — no real-time/online test yet.

**6. Connection to My Argument:** This is a concrete example of hybrid traditional+deep-learning fusion as a possible extension of my own FBCCA pipeline. Its paired t-test approach (p<0.05) is also a useful model for how I could statistically validate my own SSVEP results in future work.

---

## 14. Breaking the Performance Barrier in Deep Learning-Based SSVEP-BCIs: A Joint Frequency-Phase Training Strategy
*Ding, Liu & Chen (2026), Journal of Neural Engineering*

**1. Problem Addressed:** Existing deep learning training strategies for SSVEP use either frequency information or phase information, but not both together — this limits classification accuracy, especially at very short data windows.

**2. Method/Algorithm:** Joint Frequency-Phase Training Strategy (JFPTS), a two-stage training method: stage 1 uses a frequency-prior sampling scheme to use frequency information better; stage 2 uses a phase-locked sampling scheme to keep timing consistent within each target class. Applied mainly to a Transformer-based model called t-SSVEPformer.

**3. Key Result:** At a 0.3 s window, the JFPTS-enhanced model beat TDCA (a strong traditional baseline) for about 91% of subjects on the Benchmark dataset and 90% on BETA, and beat TRCA for almost all subjects. JFPTS reached 76.83% accuracy (Benchmark) and 64.71% (BETA), clearly higher than frequency-only training (63.25%/60.03%) or phase-only training (45.96%/28.21%) used alone.

**4. Robustness Claim:** JFPTS also improved performance when applied to five other deep learning architectures (EEGNet, tCNN, CNN-Former, C-CNN, SSVEPformer), consistently beating single-scheme training. However, the ideal balance between the two training stages needs to be tuned for each specific model and dataset.

**5. Stated Limitation (authors' own words):** On the noisier BETA dataset, several models actually performed better with frequency-only training than with the full two-stage JFPTS — meaning phase consistency helps less under noisier, more realistic conditions. The authors suggest future work on phase-preserving data augmentation to reduce overfitting in the phase-locked training stage.

**6. Connection to My Argument:** This is a very recent (2026) example of a training-strategy-level robustness technique, rather than a new architecture or hand-designed feature. It is a useful reminder, for my adversarial/robustness framing, that even the newest deep learning methods still trade phase-based gains against noise robustness — no single fix solves all three robustness dimensions at once.

---

## Summary Table (for quick reference)

| # | Paper | Category (Zerafa taxonomy) | Best reported ITR |
|---|---|---|---|
| 1 | Vialatte et al. 2010 (SSVEP review) | — (foundational) | 100+ bits/min (SSVEP-BCI, general) |
| 2 | Nicolas-Alonso & Gomez-Gil 2012 (BCI review) | — (foundational) | 60–100 bits/min (VEP, general) |
| 3 | Lin & Zhang 2007 (CCA) | Training-free | Not directly stated (accuracy-focused) |
| 4 | MSI 2014 | Training-free | 19.43 bits/min (online) |
| 5 | FBCCA (Chen et al.) | Training-free | 151.18 bits/min (online) |
| 5.2 | Chen et al. 2015 JFPM (PNAS) | Training-free | 4.45–4.50 bps (≈267 bits/min) |
| 6 | TRCA (Nakanishi et al.) | Subject-specific | 325.33 bits/min (online, cued) |
| 7 | Zerafa et al. 2018 (survey) | — (taxonomy source) | — |
| 8 | EEGNet | Subject-specific / cross-subject | Not ITR-focused |
| 9 | Traditional vs. DL survey | — (field-level survey) | 94.85% acc. (zero-cal., FB-CCNN) |
| 10 | Square wave attack | — (attack study) | — |
| 11 | Zhang & Tao 2026 (DL narrative review) | Mixed | 251.54 bits/min (lightweight, 0.4 s) |
| 12 | SUTL (Li et al. 2024) | Subject-independent | 265.31 bits/min (0.8 s) |
| 13 | eTRCA+sbCNN (Wei et al. 2025) | Subject-specific + DL hybrid | 197.48 bits/min |
| 14 | JFPTS (Ding et al. 2026) | Subject-specific / DL | Accuracy-focused, not ITR-headline |

---

**Reminder for research integrity:** Numbers marked ⚠️ above still need your own check against the original figure or supplementary file before you put them in the final report. Everything else in this document was read directly from the source PDFs during this session.
