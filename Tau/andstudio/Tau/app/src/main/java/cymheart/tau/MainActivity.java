package cymheart.tau;

import androidx.appcompat.app.AppCompatActivity;
import android.Manifest;
import android.content.Context;
import android.os.Bundle;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.style.ImageSpan;
import android.util.Log;
import android.view.View;
import android.widget.Button;
//import com.tbruyelle.rxpermissions3.RxPermissions;

import com.tbruyelle.rxpermissions3.RxPermissions;

import cymheart.tau.FX.Equalizer;
import cymheart.tau.FX.SpectrumVisual;

public class MainActivity extends AppCompatActivity   {

    Tau tau;
    Tau prevtau;
    SoundFont sf;
    VirInstrument virInst;
    Equalizer eq;
    Context context;

    private static final String[] permissionsGroup=
            new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE};


//    class ButtonListener implements View.OnClickListener, View.OnTouchListener {
//
//        public void onClick(View v) {
//            if(v.getId() == R.id.button2){
//                Log.d("test", "cansal button ---> click");
//            }
//        }
//
//        public boolean onTouch(View v, MotionEvent event) {
//            if(v.getId() == R.id.button2){
//                if(event.getAction() == MotionEvent.ACTION_UP){
//                    tau.OffKey(60, 127, virInst);
//                }
//                if(event.getAction() == MotionEvent.ACTION_DOWN){
//                    tau.OnKey(60, 127, virInst);
//                }
//            }
//
//            if(v.getId() == R.id.button3){
//                if(event.getAction() == MotionEvent.ACTION_UP){
//                    tau.OffKey(40, 127, virInst);
//                }
//                if(event.getAction() == MotionEvent.ACTION_DOWN){
//                    tau.OnKey(40, 127, virInst);
//                }
//            }
//
//            if(v.getId() == R.id.button4){
//                if(event.getAction() == MotionEvent.ACTION_UP){
//                    tau.OffKey(90, 127, virInst);
//                }
//                if(event.getAction() == MotionEvent.ACTION_DOWN){
//                    tau.OnKey(90, 127, virInst);
//                }
//            }
//
//            if(v.getId() == R.id.button5){
//                if(event.getAction() == MotionEvent.ACTION_UP){
//                    tau.OffKey(110, 127, virInst);
//                }
//                if(event.getAction() == MotionEvent.ACTION_DOWN){
//                    tau.OnKey(110, 127, virInst);
//                }
//            }
//            return false;
//        }
//
//    }

    void cb(VirInstrument[] virInst)
    {
        //cmd.OnKey(60, 127, virInst[0]);

        int s = virInst.length;

    }

    protected double[] ampOfBars = new double[1000];
    protected int[] freqOfBars = new int[1000];
    AppCompatActivity activityw;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        Button btn = findViewById(R.id.button);

        // 从布局文件中获取名叫tv_marquee的文本视图
        // 给tv_marquee设置点击监听器
        //textView.setOnClickListener(this);

       // textView.setFocusable(true); // 允许获得焦点
       // textView.setFocusableInTouchMode(true); // 允许在触摸时获得焦点
       // textView.requestFocus(); // 强制获得焦点，让跑马灯滚起来

                SpannableStringBuilder base = new SpannableStringBuilder("dfasfsdf sdfs adfsdfsddfsfsfsdff dfasdfasd sadf从布局文件中获取名叫tv_marquee的文本视图 asfas asd fsdf asf");
        int len = base.length() - 1;

       // base.insert(offset, "\u2026");
      //  base.setSpan(new RelativeSizeSpan(1), 0, len, Spanned.SPAN_INCLUSIVE_INCLUSIVE);
       // base.setSpan(new RelativeSizeSpan(2), 10, len, Spanned.SPAN_INCLUSIVE_INCLUSIVE);
       // base.setSpan(new RelativeSizeSpan(2), 10, len, Spanned.SPAN_INCLUSIVE_INCLUSIVE);

        ImageSpan imageSpan = new ImageSpan(this, R.drawable.ic_launcher_background);
        base.setSpan(imageSpan, 33, 36, Spanned.SPAN_INCLUSIVE_EXCLUSIVE);

        //ForegroundColorSpan foregroundColorSpan = new ForegroundColorSpan(Color.RED);
      //  base.setSpan(foregroundColorSpan, 12, len, Spannable.SPAN_INCLUSIVE_INCLUSIVE) ;

       // textView.setText(base);

       // RxPermissions permissions = new RxPermissions(this);
       // permissions.setLogging(true);
      //  permissions.request(permissionsGroup);

        context = this;

            System.loadLibrary("tau");

        sf = new SoundFont();
        sf.Parse("SF2", "/storage/emulated/0/gnusmas_gm_soundfont_2.01.sf2");
      //  sf.EnablePhysicsPiano(0,1,0);
        tau = new Tau(this);


      //  eq = new Equalizer();
       // Filter[] filters = eq.GetFilter();

//        Bode bode = new Bode();
//        bode.AddFilters(filters);
//
//        bode.SetSampleFreq(44100);
//        bode.SetFreqzSampleCount(500);
//
//        bode.SetPlotAreaHeight(1080);
//        bode.SetPlotAreaWidth(354);
//
//        bode.SetPlotFreqAxisStart(0);
//        bode.SetPlotFreqAxisEnd(44100/2);
//
//        bode.SetPlotGainDbRange(30);
//
//        bode.Compute();

       // float[] xpos = bode.GetPlotFreqAxisPos();
       // float[] ypos = bode.GetPlotGainAxisPos();


        //eqCmd = new EffectEqualizerCmd(ventrue, eq);
        // ventrueCmd.AddEffect(eq);

       // eqCmd.SetFreqBandGain(0, 1);

//        tau.SetFrameSampleCount(256);
//        tau.SetSampleProcessRate(44100);
//        tau.SetChannelCount(2);
//        tau.SetSoundFont(sf);
//        tau.SetUnitProcessMidiTrackCount(20);
//        tau.SetEnableAllVirInstEffects(false);
//        tau.SetLimitRegionSounderCount(64);
//        tau.SetLimitOnKeySpeed(100);
//        tau.SetEnableMidiEventCountOptimize(true);
//        tau.SetMidiKeepSameTimeNoteOnCount(10);

        tau.SetSoundFont(sf);
        tau.SetFrameSampleCount(256);
        tau.SetSampleProcessRate(44100);
        tau.SetChannelCount(2);
        tau.SetLimitRegionSounderCount(64);
        tau.SetLimitOnKeySpeed(100);
        tau.SetEnableMidiEventCountOptimize(true);
        tau.SetMidiKeepSameTimeNoteOnCount(10);
        tau.ConntectMidiDevice(0);
       // tau.SetSampleStreamCacheSec(5);
        tau.SetEnableAllVirInstEffects(false);
       // tau.SetEnableCreateFreqSpectrums(false, 4096);
      //  tau.AppendReplaceInstrument(0,0,0,0,1,0);







//        Reverb reverb = new Reverb();
//        reverb.SetRoomSize(0.7f);
//        reverb.SetWidth(0.5f);
//        reverb.SetDamping(0.3f);
//        reverb.SetEffectMix(0.5f);
//        tau.AddEffect(reverb);
        tau.Open();

        SpectrumVisual spectrumVisual = new SpectrumVisual();
        spectrumVisual.Init(40,  40, 44100, 2,6000, 160, 5120, 3);

//
//        tau.Load("/storage/emulated/0/(ACG)芬兰梦境乐团-The Dawn《魔兽世界》亡灵序曲.mid");
//        tau.Play();
//
//        int i = 250000000;
//        while(--i>0) {
//            try {
//                Thread.sleep(16);
//            } catch (InterruptedException e) {
//                throw new RuntimeException(e);
//            }
//
//          //  tau.editor.GetSpectrumsBars(ampOfBars, freqOfBars);
//
//            Log.d("s", "W");
//        }


//        tau.Load("/storage/emulated/0/QianQianQueGe.mid", true);
//
//        try {
//
//
//            Thread.sleep(5000);
//        } catch (InterruptedException e) {
//            e.printStackTrace();
//        }
//
//        tau.Play();
       // eq = new Equalizer();
       // Filter[] filters = eq.GetFilter();
        

      //  tau.Load("/storage/emulated/0/紅蓮の弓矢.mid", true);
      //  tau.Play();







//        ButtonListener b = new ButtonListener();
//
//        Button btn2 = findViewById(R.id.button2);
//      //  btn2.setOnTouchListener(b);
//
//        Button btn3 = findViewById(R.id.button3);
//        btn3.setOnTouchListener(b);
//
//        Button btn4 = findViewById(R.id.button4);
//        btn4.setOnTouchListener(b);
//
//        Button btn5 = findViewById(R.id.button5);
//        btn5.setOnTouchListener(b);


        // tv.setText(stringFromJNI());
        activityw = this;
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view)
            {
                RxPermissions permissions = new RxPermissions(activityw);
                permissions.setLogging(true);
                permissions.request(permissionsGroup)
                        .subscribe(
                                granted -> {
                                    if (granted) { // Always true pre-M
                                        // I can control the camera now
                                        //tau.ParseSoundFont("SF2", "/storage/emulated/0/gnusmas_gm_soundfont_2.01.sf2");

                                         //cmd.AppendMidiFile("/storage/emulated/0/(ACG)芬兰梦境乐团-The Dawn《魔兽世界》亡灵序曲.mid");

                                        //VirInstrument inst = tau.EnableVirInstrument(0, 0, 0, 25);
                                        //tau->SetVirInstrumentMidiControllerValue(inst, MidiControllerType::ModulationWheelMSB, 127);
                                       // tau.OnKey(60, 127, inst);


                                        //for(int i=0; i<1; i++) {
                                          //  tau.Load("/storage/emulated/0/Just the Way You Are-Bruno Mars.mid", true, true);
                                            Log.d("MidiMarker", "midiMarker:" + tau.editor.GetMidiMarkers().size());

                                            tau.editor.AddMyMarker(5);
                                        tau.editor.AddMyMarker(1);
                                        tau.editor.AddMyMarker(7);
                                      //  tau.editor.SaveMidiExInfo();

//                                            while (!tau.IsLoadCompleted())
//                                            {
//                                                Thread.sleep(50);
//                                            }
//
                                            tau.Play();
                                            //tau.Release();
                                           // tau.Release();
                                           // Thread.sleep(6000);
                                          //  tau.Stop();
                                      //      Thread.sleep(100);
                                     //   }


//                                        int s = tau.GetMidiState();
//                                        Log.d("state", s+"");
//                                        tau.PauseMidi();
//                                        s = tau.GetMidiState();
//                                        Log.d("state", s+"");
//
//                                        Thread.sleep(2000);
//
//                                        tau.PlayMidi();
//                                        s = tau.GetMidiState();
//                                        Log.d("state", "statew:" +s+"");
                                       // virInst = cmd.EnableVirInstrument(0,0,0,0);
                                        //cmd.OnKey(60, 127, virInst);

                                        //Ventrue.ndkTest();

                                    } else {
                                        //TestVentrue.VentruePlayTest();

                                    }
                                });
            }
        });




//     btn2.setOnClickListener(new View.OnClickListener() {
//        @Override
//        public void onClick(View view)
//        {
//            RxPermissions permissions = new RxPermissions(activityw);
//            permissions.setLogging(true);
//            permissions.request(permissionsGroup)
//                    .subscribe(
//                            granted -> {
//                                if (granted) { // Always true pre-M
//                                    tau.Load("/storage/emulated/0/dream.mid", false);
//
////                                    while (!tau.IsLoadCompleted())
////                                    {
////                                        Thread.sleep(50);
////                                    }
////
////                                    tau.Play();
//                                } else {
//                                    //TestVentrue.VentruePlayTest();
//
//                                }
//                            });
//        }
//    });


        // checkPermissionRequest(this);
        //testRequest(tv);
}

    protected void Run()
    {
//        prevtau = new Tau(context);
//        prevtau.SetFrameSampleCount(256);
//        prevtau.SetSampleProcessRate(44100);
//        prevtau.SetChannelCount(2);
//        prevtau.SetSoundFont(sf);
//        prevtau.SetUnitProcessMidiTrackCount(20);
//        prevtau.SetEnableAllVirInstEffects(false);
//        prevtau.SetLimitRegionSounderCount(64);
//        prevtau.SetLimitOnKeySpeed(100);
//        prevtau.Open();

        prevtau = new Tau(context);
        prevtau.SetSampleProcessRate(44100);
        prevtau.SetFrameSampleCount(512);
        prevtau.SetChannelCount(2);
        prevtau.SetLimitRegionSounderCount(64);
        prevtau.SetLimitOnKeySpeed(100);
        prevtau.SetSoundFont(sf);
        prevtau.Open();

    }


//    /**
//     * request����:
//     * ��֧�ַ���Ȩ����;
//     * ���ص�Ȩ�޽��:ȫ��ͬ��ʱֵtrue,����ֵΪfalse
//     */
//    public void testRequest(View view) {
//        RxPermissions rxPermissions = new RxPermissions(this);
//        rxPermissions.request(permissionsGroup)
//                .subscribe(granted -> {
//                    Log.i("cxw","������:"+granted);
//                });
//
//    }
//
//    public void checkPermissionRequest(FragmentActivity activity) {
//        RxPermissions permissions = new RxPermissions(activity);
//        permissions.setLogging(true);
//        permissions.request(Manifest.permission.READ_EXTERNAL_STORAGE)
//                .subscribe(
//                        granted -> {
//                    if (granted) { // Always true pre-M
//                        // I can control the camera now
//                     //   TestVentrue.VentruePlayTest();
//
//                    } else {
//                      //  TestVentrue.VentruePlayTest();
//
//                    }
//                });
//    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    //public native String stringFromJNI();





}
