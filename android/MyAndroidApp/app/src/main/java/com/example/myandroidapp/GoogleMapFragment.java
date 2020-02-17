package com.example.myandroidapp;


import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.fragment.app.Fragment;

import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.OnMapReadyCallback;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.MarkerOptions;

public class GoogleMapFragment extends Fragment implements OnMapReadyCallback {

    private static final String TAG = "MapFragment";

    private GoogleMap mMap;

    public GoogleMapFragment() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        SupportMapFragment smf = SupportMapFragment.newInstance();
        if (smf == null) {
            getChildFragmentManager().beginTransaction().add(R.id.support_map_fragment, smf, "tag").commit();// findFragmentById(R.id.support_map_fragment);
        }
        smf.getMapAsync(this);
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.fragment_googlemap, container, false);
    }

    @Override
    public void onMapReady(GoogleMap googleMap) {
        mMap = googleMap;

        // pull from sdk tutorial
        // Add a marker in Sydney, Australia, and move the camera.
        LatLng sydney = new LatLng(-34, 151);
        mMap.addMarker(new MarkerOptions().position(sydney).title("Marker in Sydney"));
        mMap.moveCamera(CameraUpdateFactory.newLatLng(sydney));
    }
}
