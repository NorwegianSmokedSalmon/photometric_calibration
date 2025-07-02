//
//  ResponseModel.h
//  OnlinePhotometricCalibration
//
//  Created by Paul on 16.11.17.
//  Copyright (c) 2017-2018 Paul Bergmann and co-authors. All rights reserved.
//
//  See LICENSE.txt
//

#ifndef OnlinePhotometricCalibration_ResponseModel_h_
#define OnlinePhotometricCalibration_ResponseModel_h_

#include "StandardIncludes.h"

/**
 * Represents data needed for the Grossberg camera response model using the first 4 basis functions
 */
class ResponseModel
{
public:
    
    /**
     * Constructor
     * 
     * Initialize response model with Grossberg parameters that correspond to identity response
     * Also initialize discretely sampled inverse response vector for 256 output intensity values
     */
    ResponseModel()
    {
        m_inverse_response_vector.clear();
        for(int i = 0;i < 256;i++)
        {
            m_inverse_response_vector.push_back(i);
        }
        
        m_grossberg_parameters.clear();
        m_grossberg_parameters.push_back(6.1);
        m_grossberg_parameters.push_back(0.0);
        m_grossberg_parameters.push_back(0.0);
        m_grossberg_parameters.push_back(0.0);
    }
    //响应模型RGB构造函数**********************************************************************need change
    ResponseModel(int ch)
    {   
        for (int ch = 0; ch < 3; ++ch)
        {
            m_inverse_response_vector[ch].resize(256);
            for (int i = 0; i < 256; ++i)
            {
                m_inverse_response_vector[ch][i] = static_cast<double>(i);
            }

            m_grossberg_parameters[ch].resize(4);
            m_grossberg_parameters[ch][0] = 6.1;
            m_grossberg_parameters[ch][1] = 0.0;
            m_grossberg_parameters[ch][2] = 0.0;
            m_grossberg_parameters[ch][3] = 0.0;
        }
    }
    
    /**
     * Apply inverse response function to output intensity 
     *
     * @param o Output intensity {0,1,...,255}
     * @returns Inverse response applied to o i.e. f^(-1)(o)
     */
    double removeResponse(int o)
    {
        return m_inverse_response_vector.at(o);
    }
    
    // RGB三通道，从像素值还原成真实的辐射值
    double removeResponseRGB(int o, int channel) const
    {
        if (channel < 0 || channel >= 3 || o < 0 || o > 255)
            return 0.0;
        return m_inverse_response_vector[channel].at(o); //查表
    }


    /**
     * Overwrite Grossberg parameter vector
     */
    // Todo: change parameter to ref
    void setGrossbergParameterVector(std::vector<double> params)
    {
        m_grossberg_parameters = params;
    }
    
    /**
     * Fetch Grossberg parameter vector
     */
    std::vector<double> getResponseEstimate()
    {
        return m_grossberg_parameters;
    }
    
    /**
     * Overwrite inverse response function vector
     */
    /*******************************************************here need change**********************************************************************/
    void setInverseResponseVector(double* new_inverse)
    {
        for(int i = 0;i < 256;i++)
        {
            m_inverse_response_vector.at(i) = 255.0 * new_inverse[i];
        }
    }

private:
    
    /**
     * Discrete vector of 256 values representing the sampled inverse camera response function
     * for the 256 image output intensities.
     * This vector is stored here for efficiency since it is not straightforward to invert the response 
     * given only the Grossberg parameters.
     */
    std::vector<double> m_inverse_response_vector;
    
    /**
     * Grossberg parameter vector (4 values) representing the camera response
     */
    std::vector<double> m_grossberg_parameters;

    //3通道RGB
    // 每个通道一个逆响应曲线和参数向量
    std::vector<double> m_inverse_response_vector[3];       // B, G, R
    std::vector<double> m_grossberg_parameters[3];          // B, G, R

};

#endif // include guard
