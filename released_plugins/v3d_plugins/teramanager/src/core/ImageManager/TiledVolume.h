//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#ifndef _TILED_VOLUME_H
#define _TILED_VOLUME_H

//#include "IM_defs.h"
//#include "MyException.h"
#include "VirtualVolume.h" // ADDED
#include <list>
#include <string>

#define TILED_FORMAT "Tiled" // ADDED

//FORWARD-DECLARATIONS
class  Block;

//******* ABSTRACT TYPES DEFINITIONS *******
#include "StackedVolume.h" 


//every object of this class has the default (1,2,3) reference system
class TiledVolume : public VirtualVolume
{
	private:	
		//******OBJECT ATTRIBUTES******
		uint16 N_ROWS, N_COLS;			//dimensions (in stacks) of stacks matrix along VH axes
        Block ***BLOCKS;			    //2-D array of <Block*>
        ref_sys reference_system;       //reference system of the stored volume
        float  VXL_1, VXL_2, VXL_3;     //voxel dimensions of the stored volume

		//***OBJECT PRIVATE METHODS****
		TiledVolume(void);

		//Given the reference system, initializes all object's members using stack's directories hierarchy
        void init();

		//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
		void rotate(int theta);

		//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
		void mirror(axis mrr_axis);

		//extract spatial coordinates (in millimeters) of given Stack object reading directory and filenames as spatial coordinates
		void extractCoordinates(Block* stk, int z, int* crd_1, int* crd_2, int* crd_3);

		// iannello returns the number of channels of images composing the volume
		void initChannels ( ) throw (MyException);

	public:
		//CONSTRUCTORS-DECONSTRUCTOR
		TiledVolume(const char* _root_dir)  throw (MyException);
        TiledVolume(const char* _root_dir, ref_sys _reference_system,
					float _VXL_1, float _VXL_2, float _VXL_3, 
					bool overwrite_mdata = false, bool save_mdata=true)  throw (MyException);

		~TiledVolume(void);

		//GET methods
		Block*** getBLOCKS(){return BLOCKS;}
        uint16 getN_ROWS(){return N_ROWS;}
        uint16 getN_COLS(){return N_COLS;}
        int    getStacksHeight();
        int    getStacksWidth();
        float  getVXL_1(){return VXL_1;}
        float  getVXL_2(){return VXL_2;}
        float  getVXL_3(){return VXL_3;}
        axis   getAXS_1(){return reference_system.first;}
        axis   getAXS_2(){return reference_system.second;}
        axis   getAXS_3(){return reference_system.third;}


		//PRINT method
		void print();

		//saving-loading methods to/from metadata binary file
		void save(char* metadata_filepath) throw (MyException);
		void load(char* metadata_filepath) throw (MyException);

		//loads given subvolume in a 1-D array of REAL_T while releasing stacks slices memory when they are no longer needed
		inline REAL_T *loadSubvolume_to_REAL_T(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (MyException) {
			return loadSubvolume(V0,V1,H0,H1,D0,D1,0,true);
		}

        //loads given subvolume in a 1-D array and puts used Stacks into 'involved_stacks' iff not null
        REAL_T *loadSubvolume(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1,
                                                                  std::list<Block*> *involved_blocks = 0, bool release_blocks = false)  throw (MyException);

        //loads given subvolume in a 1-D array of uint8 while releasing stacks slices memory when they are no longer needed
        uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1, int *channels=0)
																										throw (MyException);

		//saves given subvolume as a stack of 8-bit grayscale images in a directory created in the default path
		static void saveSubVolume(REAL_T* subvol, int V0, int V1, int H0, int H1, int D0, int D1, int V_idx, int H_idx, int D_idx);

		//saves in 'dir_path' the selected subvolume in stacked format, with the given stacks dimensions
		void saveVolume(const char* dir_path, uint32 max_slice_height=0, uint32 max_slice_width=0, 
			            uint32 V0=0, uint32 V1=0, uint32 H0=0, uint32 H1=0, uint32 D0=0, uint32 D1=0, 
						const char* img_format=IM_DEF_IMG_FORMAT, int img_depth=IM_DEF_IMG_DEPTH) throw (MyException);

		//saves in 'dir_path' the selected subvolume in multi-stack format, with the exact given stacks dimensions and the given overlap between adjacent stacks
		void saveOverlappingStacks(char* dir_path, uint32 slice_height, uint32 slice_width,	uint32 overlap_size,
						uint32 V0=0, uint32 V1=0, uint32 H0=0, uint32 H1=0, uint32 D0=0, uint32 D1=0) throw (MyException);

		//saves the Maximum Intensity Projections (MIP) of the selected subvolume along the selected direction into the given paths
		void saveMIPs(bool direction_V = true, bool direction_H = true, bool direction_D = true, 
			          char* MIP_V_path = NULL, char* MIP_H_path = NULL, char* MIP_D_path = NULL,
					  uint32 V0=0, uint32 V1=0, uint32 H0=0, uint32 H1=0, uint32 D0=0, uint32 D1=0) throw (MyException);

		//shows the selected slice with a simple GUI
		//void show(REAL_T *vol, int vol_DIM_V, int vol_DIM_H, int D_index, int window_HEIGHT=0, int window_WIDTH=0);

		//deletes slices from disk from first_file to last_file, extremes included
		//void deleteSlices(int first_file, int last_file);

		//releases allocated memory of stacks
		void releaseStacks(int first_file=-1, int last_file=-1);

		//returns true if file exists at the given filepath
		static bool fileExists(const char *filepath);

		// OPERATIONS FOR STREAMED SUBVOLUME LOAD 

		/* start a streamed load operation: returns an opaque descriptor of the streamed operation
		 * buf is a dynamically allocated, initialized buffer that should not be neither manipulated 
		 * nor deallocated by the caller until the operation terminates 
		 * (see close operations for details)
		 */
        void *streamedLoadSubvolume_open ( int steps, uint8 *buf, int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1 );

		/* perform one step of a streamed operation: returns a pointer to a read-only buffer 
		 * with updated data; the returned buffer should not be deallocated
		 * the optional parameter buffer2 is an initialized buffer of the same dimensions of 
		 * the returned buffer with reference data to be used to check that the returned buffer 
		 * is updated with exactly the same data contained in buffer2
		 * if the default value of buffer2 (null pointer) is passed no check is performed
		 */
        uint8 *streamedLoadSubvolume_dostep ( void *stream_descr, unsigned char *buffer2=0 );

		/* close a streamed load operation: by default return the initial buffer that can be 
		 * freely used and must be deallocated 
		 * if return_buffer is set to false, the initial buffer is deallocated, it cannot be 
		 * resuded by the caller and the operation returns a null pointer
		 */
         uint8 *streamedLoadSubvolume_close ( void *stream_descr, bool return_buffer=true );

};

#endif //_TILED_VOLUME_H
