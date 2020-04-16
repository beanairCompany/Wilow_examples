package com.example.lowdutycyclemqttclient.ui.main;

import android.content.Context;
import android.util.Log;

import androidx.annotation.FractionRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentPagerAdapter;

import com.example.lowdutycyclemqttclient.Fragment_channel_x;
import com.example.lowdutycyclemqttclient.Fragment_channel_y;
import com.example.lowdutycyclemqttclient.Fragment_channel_z;
import com.example.lowdutycyclemqttclient.Fragment_inclinometer_x;
import com.example.lowdutycyclemqttclient.Fragment_inclinometer_y;
import com.example.lowdutycyclemqttclient.R;

/**
 * A [FragmentPagerAdapter] that returns a fragment corresponding to
 * one of the sections/tabs/pages.
 */
public class SectionsPagerAdapter extends FragmentPagerAdapter {

    @StringRes
    private static final int[] TAB_TITLES = new int[]{R.string.tab_ch_z, R.string.tab_ch_x, R.string.tab_ch_y, R.string.tab_inc_x, R.string.tab_inc_y};
    private final Context mContext;

    public SectionsPagerAdapter(Context context, FragmentManager fm) {
        super(fm);
        mContext = context;
    }

    @Override
    public Fragment getItem(int position) {
        // getItem is called to instantiate the fragment for the given page.
        // Return a PlaceholderFragment (defined as a static inner class below).
        //return PlaceholderFragment.newInstance(position + 1);
        Fragment fragment = null;
        switch (position){
            case 0:
                fragment = new Fragment_channel_z();
                break;
            case 1:
                fragment = new Fragment_channel_x();
                break;
            case 2:
                fragment = Fragment_channel_y.newInstance();
                break;
            case 3:
                fragment = Fragment_inclinometer_x.newInstance();
                break;
            case 4:
                fragment = Fragment_inclinometer_y.newInstance();
                break;
        }
        return fragment;
    }

    @Override
    public int getItemPosition(@NonNull Object object) {
        return POSITION_NONE;
    }

    @Nullable
    @Override
    public CharSequence getPageTitle(int position) {
        return mContext.getResources().getString(TAB_TITLES[position]);
    }

    @Override
    public int getCount() {
        return 5;
    }
}