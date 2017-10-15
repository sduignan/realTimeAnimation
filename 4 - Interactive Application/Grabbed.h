#pragma once
#include <map>
#include <string>

std::map<std::string, float> grabMapping;

void setGrabMapping() {
	grabMapping["wrist_bits"] = 0.0;
	grabMapping["thumb_metacarpal"] = 32.0;
	grabMapping["thumb_p_phalange"] = 18.0;
	grabMapping["thumb_d_phalange"] = 44.75;
	grabMapping["index_metacarpal"] = 0.0;
	grabMapping["index_p_phalange"] = 100.75;
	grabMapping["index_m_phalange"] = 82.75;
	grabMapping["index_d_phalange"] = 100.75;
	grabMapping["middle_metacarpal"] = 0.0;
	grabMapping["middle_p_phalange"] = 98.75;
	grabMapping["middle_m_phalange"] = 90.75;
	grabMapping["middle_d_phalange"] = 100.75;
	grabMapping["ring_metacarpal"] = 0.0;
	grabMapping["ring_p_phalange"] = 92.75;
	grabMapping["ring_m_phalange"] = 78.75;
	grabMapping["ring_d_phalange"] = 94.75;
	grabMapping["pinky_metacarpal"] = 0.0;
	grabMapping["pinky_p_phalange"] = 90.75;
	grabMapping["pinky_m_phalange"] = 72.75;
	grabMapping["pinky_d_phalange"] = 112.75;
}