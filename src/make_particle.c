#include <math.h>
#include "const.h"
#include "cmplx.h"
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "comm.h"
#include "read_cluster.h"

/*      This module calculates the coordinates of dipoles
 *      for a sphere, for the 'fractional packing.
 *      The array coord[N][3] must contain
 *      the number of the dipoles and the coordinates. This
 *      array will be used by other tools, such as viz. and
 *      tools to calculate and examine the matrix A
 *
 *
 *________________________________________________________
 *
 *              ALfons Hoekstra, dec. 1990
 *-------------------------------------------------------
 *
 *      The function is slightly adapted, by including the
 *      wavelenght, levels, and dips_per_lambda in the
 *      function header.
 *
 *                               sept. 1992
 *-----------------------------------------------------------
 *
 * the function anint, present in the unix libraries, is written
 * out in full detail, since it is not present in this release of
 * parix (v. 1.1)
 *
 * rewriten,
 * Michel Grimminck 1995
 * included pbm creation
 * -----------------------------------------------------------
 *  included ellipsoidal particle for work with Victor Babenko
 *  september 2002
 *  --------------------------------------------------------
 *  included many more new particles:
 *  leucocyte, stick, rotatable oblate spheroid, lymphocyte, 
 *  rotatable RBC, etc etc etc
 *  December 2003 - February 2004, by Konstantin Semianov
 */


double centreX,centreY,centreZ;

extern int boxX,boxY,boxZ;
extern int nDip; 		/* defined in calculator */
extern int RingId;
extern short int *position;
extern int symX,symY,symZ,symD,symR;
extern double gridspaceX,gridspaceY,gridspaceZ;

extern FILE *logfile;
extern double LAMBDA;
extern double dplX,dplY,dplZ;
extern double DIPS_PER_LAMBDA;
extern char aggregate_file[];
extern REAL scale_fac;

double
my_anint (double x)
{
  double y;
  
  y = ceil (x);
  if ((y - x) <= 0.5) {
    return (y);
  }
  else {
    return (floor(x));
  }
}


int
make_particle (REAL **coord,	/* coordinate vector */
	       int store,           /* store in coord table (true,false) */
	       int shape,           /* particle shape */
	       int jagged)
     /* generate a particle and determine its symmetries: mirror X,
      * mirror Y,mirror Z, mirror XY,rotation 90 degrees. Estimate
      * what the volume of the particle is, when not discretisized.
      */
{
  int i, j, k, dipcount,index,InPart;
  double x, y, z;
  double volume;
  double radiussq;
  double xr,yr,zr,xcoat,ycoat,zcoat;
  double r_eps, delta1, thet_, rr, phi_, deltaphi;
  int n, ni, N, ii,ij,ik;
  double xr_,yr_,zr_,CX,CY,SX,SY,CZ,SZ, bradius, cradius, clength, cthick, x_,y_,z_;
  double rrc0, rrc1, rrc2, rrc3, rrc4, rrc5, radius0, radius1, radius2, radius3, radius4, radius5, 
         x_incl, y_incl, z_incl;
  int xj,yj,zj;
  int mat;
  extern int Nmat;
  
  extern int NoSymmetry;
  extern int *material;
  extern double coat_ratio,coat_x,coat_y,coat_z;
  extern double append_ratio, ratio_x, ratio_y, ratio_z, cratio_x, cratio_y, cratio_z;
  extern double xc0,yc0,zc0,xc1,yc1,zc1,xc2,yc2,zc2,xc3,yc3,zc3,xc4,yc4,zc4,
  ratio0, ratio1, ratio2, ratio3, ratio4, ratio5, delta, inc_ratio;
  extern double diskratio;
  extern double ellipsX,ellipsY,ellipsZ;
  double dradius, radius, dthick;
  extern double aspect_r, betaY, betaZ;
  extern double aspect_ratio, alphaY, alphaX;  

  FILE *debug;
  int ngrains,grain=0;
  int *gr_mat;
  REAL *gr_rad,*gr_coord[3];
  REAL d_box,max;
  
  if (shape==ELLIPS || shape==SPHERE) {
    symX=symY=symZ=true;
    volume=PI/6*boxX*boxY*boxZ;
    if (boxX==boxY) symR=true; else symR=false;
    /* symR = false;*/
  }
  if (shape==ELLIPSOIDAL) {
    symX=symY=symZ=true;
    if (ellipsX==ellipsY) symR=true; else symR=false;
  }
  else if(shape==DISK) {
    symX=symY=symZ=false;
    symR=false;
    volume=PI/4*boxX*boxX*boxX*diskratio;
  }
  else if(shape==SDISK) {
    symX=symY=symZ=false;
    symR=false;
    volume=PI/4*boxX*boxX*boxX*diskratio;
  }
  else if(shape==RBC) {
    symX=symY=symZ=false;
    symR=false;
    volume=boxX*boxX*boxX;
  }
  else if(shape==RBC_ROT) {
    symX=symY=symZ=false;
    symR=false;
    volume=boxX*boxX*boxX;
  }
  else if(shape==SDISK_ROT) {
    symX=symY=symZ=false;
    symR=false;
    volume=boxX*boxX*boxX;
  }
  else if(shape==SPHEROID_ROT) {
    symX=symY=symZ=false;
    symR=false;
    volume=boxX*boxX*boxX;
  }
  else if(shape==CYLINDER) {
    symX=symY=symZ=false;
    symR=false;
    volume=boxX*boxX*boxX;
  }   
  else if(shape==STICK) {
    symX=symY=symZ=true;
    symR=false;
    if ((alphaX==0) && (alphaY==0)) {symR=true;}
    volume=boxX*boxX*boxX;
  }       
  else if (shape==LYMPHOCYTE1) {
    if (Nmat<2) printz("WARNING: only one refraction index is given\n");
    symX=symY=symZ=false;
    symR=false;
    volume=boxX*boxX*boxX;
  }
  else if (shape==LYMPHOCYTE2) {
    if (Nmat<2) printz("WARNING: only one refraction index is given\n");
    symX=symY=symZ=true;
    symR=true;
    if (coat_x!=0 || cratio_x!=1 || ratio_x!=1) {symX=false; symR=false;}
    if (coat_y!=0 || cratio_y!=1 || ratio_y!=1) {symY=false; symR=false;}
    if (coat_z!=0 || cratio_z!=1 || ratio_z!=1) symZ=false;
    volume=boxX*boxX*boxX;
  }
  else if (shape==LEUCOCYTE2) {
    if (Nmat<3) printz("WARNING: only one refraction index is given\n");
    symX=symY=symZ=false;
    symR=false;
    volume=boxX*boxX*boxX;
  }
  
  else if (shape==COATED) {
    if (Nmat<2) printz("WARNING: only one refraction index is given\n");
    symX=symY=symZ=true;
    volume=PI/6*boxX*boxY*boxZ;
    if (boxX==boxY) symR=true; else symR=false;
    if (coat_x!=0) {symX=false; symR=false;}
    if (coat_y!=0) {symY=false; symR=false;}
    if (coat_z!=0) symZ=false;
  }
  else if (shape==SPHEREBOX) {
    if (Nmat<2) printz("WARNING: only one refraction index is given\n");
    symX=symY=symZ=true;
    volume=boxX*boxY*boxZ;
    if (boxX==boxY) symR=true; else symR=false;
    if (coat_x!=0) {symX=false; symR=false;}
    if (coat_y!=0) {symY=false; symR=false;}
    if (coat_z!=0) symZ=false;
  }
  else if (shape==BOX) {
    symX=symY=symZ=true;
    volume=boxX*boxY*boxZ;
    if (boxX==boxY) symR=true; else symR=false;
  }
  else if (shape==PRISMA) {
    symX=true;
    symY=symZ=false;
    symR=false;
    volume=.5*boxX*boxY*boxZ;
  }
  else if (shape==LINE) {
    symX=true;
    symY=symZ=true;
    symR=false;
    volume=0;
  }
  else if (shape==AGGREGATE)
    {
      /* Read aggregate */
      symX=symY=symZ=symR=false;
      if (store == false && ringid == 0)
	format(aggregate_file,"aggr_dummy.dat");
      synchronize();
      read_cluster("aggr_dummy.dat",&ngrains,gr_coord,&gr_rad,&gr_mat);
      if (store == true)
	{
	  setup_box(ngrains,gr_coord,&max);
	  gridspaceX = gridspaceY = gridspaceZ = 
	    pow((4*PI)/3,(double) 1/3)*gr_rad[0];
	  if (gridspaceX*boxX < 2*max)
	    {
	      printz("aggregate box-size = %g; DDA box-size = %g\n"\
		     "\tDDA-box too small\n\tIncrease grid-size\n",
		     2*max,gridspaceX*boxX);
	      stop(1);
	    }
	  DIPS_PER_LAMBDA = dplX = dplY = dplZ = LAMBDA/gridspaceX;
	  fprintz(logfile,"Aggregate characteristics\n"\
		  "\taggregate box-size: %lg\n"\
		  "\tVgrain:             %lg\n"\
		  "\tRescaled aggregate dimension\n"\
		  "\t with dimension     %lg\n"\
		  "Aggregate adjustments\n"\
		  "\tRe(m) * Dipoles/lambda:\n"\
		  "\t                    %lg\n"\
		  "\tGridspace:          %lg\n"\
		  "\tVdip:               %lg\n",
		  2*max,(4*PI)/3*pow(gr_rad[0],3),scale_fac,
		  DIPS_PER_LAMBDA,gridspaceX,pow(gridspaceX,3));
	}
    }
  
  dipcount=index=0;
  radiussq=pow(boxX/2*gridspaceX,2);
  
  if ((boxX/jagged)%2==0) {centreX=jagged/2.0;}
  else {
    if (boxX%2==0) centreX=(jagged)/2.0; else centreX=(jagged-1)/2.0;
  }
  
  if ((boxY/jagged)%2==0) {centreY=jagged/2.0;}
  else {
    if (boxY%2==0) centreY=(jagged)/2.0; else centreY=(jagged-1)/2.0;
  }
  
  if ((boxZ/jagged)%2==0) {centreZ=jagged/2.0;}
  else {
    if (boxZ%2==0) centreZ=(jagged)/2.0; else centreZ=(jagged-1)/2.0;
  }
  
  if (shape == AGGREGATE)
    {
      /* First set all gridpoints to void */
      for(k=-boxZ/2;k<=(boxZ-1)/2;k++)
	for(j=-boxY/2;j<=(boxY-1)/2;j++)
	  for(i=-boxX/2;i<=(boxX-1)/2;i++) {
	    x=(i+centreX)*gridspaceX;
	    y=(j+centreY)*gridspaceY;
	    z=(k+centreZ)*gridspaceZ;

	    mat=Nmat-1;
	    if (mat>=0) {
	      if (store==true) {
		if (dipcount>=local_d0 && dipcount<local_d1) {
		  position[3*index]=i;
		  position[3*index+1]=j;
		  position[3*index+2]=k+(boxX+1)/2; /* for PARALLEL impl. */
		  coord[index][0] = x;
		  coord[index][1] = y;
		  coord[index][2] = z;
		  material[index]=mat;
		  index++;
		}
	      }
	      dipcount++;
	    }
	  } /* End of grid-initialisation loop */


      /* Projection of the aggregate */
      if (store==true)
	{
	  int *_mat_count;

	  for (grain=0;grain<ngrains;++grain)
	    {
	      /* Determine the nearest dipole-site */
	      i = ((int) my_anint(gr_coord[0][grain]/gridspaceX
				  - centreX)) + boxX/2;
	      j = ((int) my_anint(gr_coord[1][grain]/gridspaceY
				  - centreY)) + boxY/2;
	      k = ((int) my_anint(gr_coord[2][grain]/gridspaceZ
				  - centreZ)) + boxZ/2;
	      index = i + boxX*j + boxX*boxY*(k-local_z0);

	      if (local_z0 <= k && k < local_z1)
		{
		  /* Change void to grain-material */
		  material[index] = 0;
		}
	    } /* end of grain loop */

	  /* Check the number of grains */
	  _mat_count = (int *) calloc(Nmat,sizeof(int));
	  for (i=0;i<local_Ndip;++i)
	    _mat_count[material[i]]++;
	  if (local_Ndip-_mat_count[Nmat-1] != ngrains)
	    {
	      printf("\tWhile projecting the aggregate on the dipole-grid\n"\
		     "\t %d grains got lost.\n",
		     ngrains - (local_Ndip-_mat_count[Nmat-1]));
	    }
	}

      free_aggregate(gr_coord,gr_rad,gr_mat);
    }
  else
    {
      for(k=-boxZ/2;k<=(boxZ-1)/2;k++)
	for(j=-boxY/2;j<=(boxY-1)/2;j++)
	  for(i=-boxX/2;i<=(boxX-1)/2;i++) {
	    x=(i+centreX)*gridspaceX;
	    y=(j+centreY)*gridspaceY;
	    z=(k+centreZ)*gridspaceZ;
	    
	    mat=-1;
	    if (shape==BOX) mat=0;
	    if (shape==DISK) {
	      dradius = (boxX*gridspaceX)/2.0;  /* assuming X, Y, Z is equal */
	      dthick = diskratio*dradius;
	      if(x <= 0 && x >= -dthick) {
		if(y*y + z*z <= dradius*dradius) {
		  mat = 0;
		}
	      }
	    }
	    if (shape==SDISK) {
	      dradius = (boxX*gridspaceX)/2.0;  /* assuming X, Y, Z is equal */
	      dthick = diskratio*dradius;
	      if(x <= dthick/2.0 && x >= -dthick/2.0) {
		if(y*y + z*z <= dradius*dradius) {
		  mat = 0;
		}
	      }
	    }
	    if (shape==RBC) { /* here we assume units of micrometers */
	      dradius = (boxX*gridspaceX)/2.0;  /* assuming X, Y, Z is equal */
	      radius = y*y + z*z;
	      if(radius <= dradius*dradius) {
		radius /= (dradius*dradius);
		dthick = (dradius/3.91)*sqrt(1.0-radius) *
		  (0.81 + 7.83*radius - 4.39*radius*radius);
		if(x<dthick/2.0 && x>-dthick/2.0) {
		  mat = 0;
		}
	      }
	    }
	    if (shape==RBC_ROT) { /* rottatable erithrocyte around two axes Y and Z */
	      dradius = (boxX*gridspaceX)/2.0;  /* assuming X, Y, Z is equal */
	      CY=cos(PI/180*betaY);  SY=sin(PI/180*betaY);
	      CZ=cos(PI/180*betaZ);  SZ=sin(PI/180*betaZ);
	      x_=x*CY*CZ-y*SZ-z*SY*CZ;
	      y_=x*CY*SZ+y*CZ-z*SY*SZ;
	      z_=x*SY+z*CY;
	      radius=y_*y_+z_*z_;
	      if (radius <dradius*dradius) {
		radius /= (dradius*dradius);
		dthick=(dradius*aspect_r)* sqrt(1.0-radius) * 
		   (0.81 + 7.83*radius - 4.39*radius*radius); /* for previous aspect_r=1/3.91 */
		if (x_<dthick/2.0 && x_>-dthick/2.0) { 
		  mat = 0;
		} 
	      }
            }
            if  (shape==SDISK_ROT) {
  	      xr= (i+centreX)/(boxX);
	      yr= (j+centreY)/(boxY);
	      zr= (k+centreZ)/(boxZ);
              CY=cos(PI/180*betaY);  SY=sin(PI/180*betaY);
              CZ=cos(PI/180*betaZ);  SZ=sin(PI/180*betaZ);
              xr_=xr*CY*CZ-yr*SZ-zr*SY*CZ;
              yr_=xr*CY*SZ+yr*CZ-zr*SY*SZ;
              zr_=xr*SY+zr*CY;
              cthick=(aspect_r)/2.0;
              radius=(xr_*xr_+yr_*yr_+zr_*zr_)/0.25;     
              if (radius <1.0000001 && xr_<cthick && xr_>-cthick) {
                mat=0;
              }
            }
            if  (shape==SPHEROID_ROT) {
  	      xr= (i+centreX)/(boxX);
	      yr= (j+centreY)/(boxY);
	      zr= (k+centreZ)/(boxZ);
              CY=cos(PI/180*betaY);  SY=sin(PI/180*betaY);
              CZ=cos(PI/180*betaZ);  SZ=sin(PI/180*betaZ);
              xr_=xr*CY*CZ-yr*SZ-zr*SY*CZ;
              yr_=xr*CY*SZ+yr*CZ-zr*SY*SZ;
              zr_=xr*SY+zr*CY;
	      xr_/=aspect_r;
              radius=(xr_*xr_+yr_*yr_+zr_*zr_)/0.25;     
              if (radius <1.0000001) {
		      mat=0;
              }
            }
	    if (shape==STICK) {
	       xr= (i+centreX)/(boxX);
	       yr= (j+centreY)/(boxY);
	       zr= (k+centreZ)/(boxZ);
	       CY=cos(PI/180*alphaY);  SY=sin(PI/180*alphaY);
	       CX=cos(PI/180*alphaX);  SX=sin(PI/180*alphaX);
	       xr_=xr*CY - zr*SY;
	       yr_=-xr*SY*SX+yr*CX-zr*CY*SX;
	       zr_=xr*SY*CX+yr*SX+zr*CY*CX;
	       bradius=append_ratio*append_ratio*aspect_ratio*aspect_ratio/3.99999999999;
	       clength=append_ratio*(1-aspect_ratio)/2.0;
	       radius=(yr_*yr_+xr_*xr_);
	       if ((radius <bradius && zr_<clength && zr_>-clength) || 
		   (radius+(zr_-clength)*(zr_-clength))< bradius ||  
		   (radius+(zr_+clength)*(zr_+clength))<bradius) {
		   mat = 0;
	        }
	    }
            if (shape==CYLINDER) {
	       xr= (i+centreX)/(boxX);
	       yr= (j+centreY)/(boxY);
	       zr= (k+centreZ)/(boxZ);
	       bradius=aspect_ratio*aspect_ratio*append_ratio*append_ratio/3.99999999999;
	       clength=append_ratio/2.0;
	       radius=(yr*yr+xr*xr);
	       if (radius <bradius && zr<clength && zr>-clength) {
	         mat=0;
	       };
            };
            if (shape==LYMPHOCYTE1) {
	       xr= (i+centreX)/(boxX);
	       yr= (j+centreY)/(boxY);
	       zr= (k+centreZ)/(boxZ);
	       xcoat=xr-coat_x/2;
	       ycoat=yr-coat_y/2;
	       zcoat=zr-coat_z/2;    
	       r_eps=0.25*0.25*(1-append_ratio)*(1-append_ratio);   
	       if (xr*xr+yr*yr+zr*zr < .250001*append_ratio*append_ratio) mat=0;
	       if (xcoat*xcoat+ycoat*ycoat+zcoat*zcoat <0.250001*coat_ratio*coat_ratio*append_ratio*append_ratio) mat=1;
	       if ((xr*xr+yr*yr+zr*zr >0.25000*append_ratio*append_ratio) && (xr*xr+yr*yr+zr*zr < 0.250001)) {
	          delta1=PI/12.0;
	          for (ni=0; ni<=12; ni++) {
		    thet_=ni*delta1; rr=(append_ratio+1)/4.0*sin(thet_); z_=(append_ratio+1)/4.0*cos(thet_);
		    deltaphi=2.0*PI/(23.0*(1-abs(1.0-2.0*thet_/PI))+1);
		    phi_=0;
		    while (phi_<2.0*PI) {
		      x_=rr*cos(phi_);  y_=rr*sin(phi_);
		      bradius=(xr-x_)*(xr-x_)+(yr-y_)*(yr-y_)+(zr-z_)*(zr-z_);
		      if (bradius<r_eps) mat=0;
		      phi_=phi_+deltaphi;
		    }
	          }; 
	       }
	    };
            if (shape==LYMPHOCYTE2) {
	       xr= (i+centreX)/(boxX);
	       yr= (j+centreY)/(boxY);
	       zr= (k+centreZ)/(boxZ);
	       xcoat=xr-coat_x/2;
	       ycoat=yr-coat_y/2;
	       zcoat=zr-coat_z/2;
	       radius=(xr*xr/ratio_x/ratio_x+yr*yr/ratio_y/ratio_y+zr*zr/ratio_z/ratio_z)/append_ratio/append_ratio;
	       cradius=(xcoat*xcoat/cratio_x/cratio_x+ycoat*ycoat/cratio_y/cratio_y+zcoat*zcoat/cratio_z/cratio_z)/append_ratio/append_ratio;	       
	       if (radius < 0.250001) mat=0;
	       if (cradius<0.250001*coat_ratio*coat_ratio) mat=1;
            };
             if (shape==LEUCOCYTE2) {
	        xr= (i+centreX)/(boxX);
	        yr= (j+centreY)/(boxY);
	        zr= (k+centreZ)/(boxZ);
	        N=(1/delta);
		rr=(1-inc_ratio/2.0)*(1-inc_ratio/2.0)*0.25;
		r_eps=0.25*inc_ratio*inc_ratio;
		radius=xr*xr+yr*yr+zr*zr;
		if (xc0>=1) xc0=-(xc0-1.0); if (xc1>=1) xc1=-(xc1-1.0); if (xc2>=1) xc2=-(xc2-1.0); 
		if (xc3>=1) xc3=-(xc3-1.0); if (xc4>=1) xc4=-(xc4-1.0);
		if (yc0>=1) yc0=-(yc0-1.0); if (yc1>=1) yc1=-(yc1-1.0); if (yc2>=1) yc2=-(yc2-1.0); 
		if (yc3>=1) yc3=-(yc3-1.0); if (yc4>=1) yc4=-(yc4-1.0);
		if (zc0>=1) zc0=-(zc0-1.0); if (zc1>=1) zc1=-(zc1-1.0); if (zc2>=1) zc2=-(xc2-1.0); 
		if (zc3>=1) zc3=-(zc3-1.0); if (zc4>=1) zc4=-(zc4-1.0);
		rrc0=0.25*(ratio0+inc_ratio/2.0)*(ratio0+inc_ratio/2.0);
		radius0= (xc0-xr)*(xc0-xr)+(yc0-yr)*(yc0-yr)+(zc0-zr)*(zc0-zr);
		rrc1=0.25*(ratio1+inc_ratio/2.0)*(ratio1+inc_ratio/2.0);
		radius1= (xc1-xr)*(xc1-xr)+(yc1-yr)*(yc1-yr)+(zc1-zr)*(zc1-zr);
		rrc2=0.25*(ratio2+inc_ratio/2.0)*(ratio2+inc_ratio/2.0);
		radius2= (xc2-xr)*(xc2-xr)+(yc2-yr)*(yc2-yr)+(zc2-zr)*(zc2-zr);
		rrc3=0.25*(ratio3+inc_ratio/2.0)*(ratio3+inc_ratio/2.0);
		radius3= (xc3-xr)*(xc3-xr)+(yc3-yr)*(yc3-yr)+(zc3-zr)*(zc3-zr);
		rrc4=0.25*(ratio4+inc_ratio/2.0)*(ratio4+inc_ratio/2.0);
		radius4= (xc4-xr)*(xc4-xr)+(yc4-yr)*(yc4-yr)+(zc4-zr)*(zc4-zr);
		if ( radius< 0.250001) {
		   mat=0;
		   if ( radius0<0.250001*ratio0*ratio0)  mat=1;
		   else
		   if ( radius1<0.250001*ratio1*ratio1)  mat=1;
		   else
		   if ( radius2<0.250001*ratio2*ratio2)  mat=1;
		   else
		   if ( radius3<0.250001*ratio3*ratio3)  mat=1;
		   else
		   if ( radius4<0.250001*ratio4*ratio4)  mat=1;
		   else
		   if  (radius <rr) {
		     for (ik=1; ik<N; ik++) {
		       z_incl=(zr+0.5-delta*ik)*(zr+0.5-delta*ik);
		       for (ij=1; ij<N; ij++) {
		      	 y_incl=(yr+0.5-delta*ij)*(yr+0.5-delta*ij);
			 for (ii=1; ii<N; ii++) {
			   x_incl=(xr+0.5-delta*ii)*(xr+0.5-delta*ii);
			   if  ( x_incl+y_incl+z_incl<r_eps) mat=1;
			 }
		       }
		     }
		   }
	        }
             }

	    if (shape==PRISMA && y*boxZ>=-z*boxY) mat=0;
	    if (shape==ELLIPS || shape==SPHERE) {
	      xj=jagged*((i+boxX/2)/jagged)-boxX/2;
	      yj=jagged*((j+boxY/2)/jagged)-boxY/2;
	      zj=jagged*((k+boxZ/2)/jagged)-boxZ/2;
	      
	      xr=(REAL) (xj+centreX)/(boxX);
	      yr=(REAL) (yj+centreY)/(boxY);
	      zr=(REAL) (zj+centreZ)/(boxZ);
	      
	      if (xr*xr+yr*yr+zr*zr < .250001) mat=0;
	    }
	    if (shape==ELLIPSOIDAL) {
	      if (x*x/(ellipsX*ellipsX) +
		  y*y/(ellipsY*ellipsY) + 
		  z*z/(ellipsZ*ellipsZ) < 0.25 * boxX*boxX*gridspaceX*gridspaceX) mat = 0;
	    }
	    if (shape==COATED) {
	      xj=jagged*((i+boxX/2)/jagged)-boxX/2;
	      yj=jagged*((j+boxY/2)/jagged)-boxY/2;
	      zj=jagged*((k+boxZ/2)/jagged)-boxZ/2;
	      
	      xr=(REAL) (xj+centreX)/(boxX);
	      yr=(REAL) (yj+centreY)/(boxY);
	      zr=(REAL) (zj+centreZ)/(boxZ);
	      xcoat=xr-coat_x/2;
	      ycoat=yr-coat_y/2;
	      zcoat=zr-coat_z/2;
	      if (xr*xr+yr*yr+zr*zr < .250001) mat=0;
	      if (xcoat*xcoat+ycoat*ycoat+zcoat*zcoat < .250001*coat_ratio*coat_ratio) mat=1;
	    }
	    if (shape==SPHEREBOX) {
	      xj=jagged*((i+boxX/2)/jagged)-boxX/2;
	      yj=jagged*((j+boxY/2)/jagged)-boxY/2;
	      zj=jagged*((k+boxZ/2)/jagged)-boxZ/2;
	      
	      xr=(REAL) (xj+centreX)/(boxX);
	      yr=(REAL) (yj+centreY)/(boxY);
	      zr=(REAL) (zj+centreZ)/(boxZ);
	      if (xr*xr+yr*yr+zr*zr < .250001*coat_ratio*coat_ratio) mat=1; else mat=0;
	    }
	    if (shape==LINE) {
	      if (j==0 && k==0) mat=0;
	    }
	    /*#ifdef PARALLEL*/
	    if (mat<0) mat=Nmat-1;
	    /*#endif*/
	    if (mat>=0) {
	      if (store==true) {
		if (dipcount>=local_d0 && dipcount<local_d1) {
		  position[3*index]=i;
		  position[3*index+1]=j;
		  position[3*index+2]=k+(boxX+1)/2; /* for PARALLEL impl. */
		  coord[index][0] = x;
		  coord[index][1] = y;
		  coord[index][2] = z;
		  material[index]=mat;
		  index++;
		}
	      }
	      dipcount++;
	    }
	  } /* End box loop */
    }

  if (gridspaceX!=gridspaceY) symR=false;
  if (store==false) printz("discrete volume error:%.1f%%\n",100.0*(dipcount-volume)/dipcount);
  if (NoSymmetry==true) {
    symX=symY=symZ=symR=false;
  }
  if (store==true) {setup_use(coord,dipcount,1.0/gridspaceX,
			      1.0/gridspaceY,1.0/gridspaceZ);}
  return(dipcount);
}


void pbm_fields(char which,int comp)
     /* draws picture of one component of the E field, in pbm format.
      * The particle is divided in slices through the Y-Z plane, stating
      * with the most negative X-value. The Y-direction is drawn downward,
      * the Z-direction is to the right.
      * The fase is coded in the colour of a pixel, the amplitude in its
      * intensity.
      *
      * WARNING: the program 'xv' has a bug and is not able to
      * show these files properly.
      */
{
  extern dcomplex *x;
  extern REAL **DipoleCoord;
  extern int round();
  extern char directory[200];
  extern int *material;
  
  REAL **st1;
  double ampl,fase;
  double rtemp0,rtemp1,rtemp2;
  double max=-1e308,min=1e308;
  char filename[250];
  unsigned char *bitmapr,*bitmapg,*bitmapb;
  int xx,yy,zz,rx,ry,i,j,X,Y,jjj,b;
  int Nx;
  FILE *pbm,*fields;

  strcpy(filename,directory);
  if (which=='X') strcat(filename,"/fieldX.pbm");
  else strcat(filename,"/fieldY.pbm");

  Nx=sqrt(boxX*1.5);                   /* number of slices next to eachother */
  X=Nx*(boxZ+10);                      /* width of the picture */
  Y=(boxY+10)*ceil((double) boxX/Nx);  /* height of the picture */
  bitmapr=(unsigned char *) malloc(X*Y);
  bitmapg=(unsigned char *) malloc(X*Y);
  bitmapb=(unsigned char *) malloc(X*Y);
  if (bitmapr==NULL || bitmapg==NULL || bitmapb==NULL) {
    printf("malloc failed for bitmap\n");
    return;
  }

  /* find minimum and maximum amplitude */
  j=0;
  for(i=0;i<nDip;i++) {
    ampl=sqrt(x[3*i+comp].r*x[3*i+comp].r + x[3*i+comp].i*x[3*i+comp].i);
    if (ampl<min) min=ampl;
    if (ampl>max) max=ampl;
  }
  
  /* set up file */
  pbm=fopen(filename,"w");
  if (pbm==NULL) {printf("field file error\n"); return;}
  strcpy(filename,directory);
  if (which=='Y') strcat(filename,"/E_fieldY");
  if (which=='X') strcat(filename,"/E_fieldX");
  
  fields=fopen(filename,"w");
  if (fields==NULL) {printz("Efield file error\n"); return;}
  fprintz(fields,"%i\n",nDip);
  fprintz(pbm,"P6\n");
  fprintz(pbm,"# field component:%i\n",comp);
  fprintz(pbm,"# minimal amplitude:%g\n",min);
  fprintz(pbm,"# maximal amplitude:%g\n",max);
  fprintz(pbm,"%i %i\n",X,Y);
  fprintz(pbm,"255\n");

  /* draw background */
  for(i=0;i<X*Y;i++) {
    bitmapr[i]=124;
    bitmapg[i]=51;
    bitmapb[i]=37;
  }
  
  /* draw slices */
  st1=DipoleCoord;
  for(i=0;i<nDip;i++) {
    jjj=i*3+comp;
    
    xx=position[3*i]; yy=position[3*i+1]; zz=position[3*i+2];
    xx+=boxX/2; yy+=(boxY+10)/2; zz+=(boxZ+10)/2;
    ry=(xx/Nx)*(boxY+10)+yy;
    rx=(xx%Nx)*(boxZ+10)+zz;
    b=rx+X*ry;
    
    fase=(atan2(x[jjj].r,x[jjj].i)+PI)/(2*PI);
    ampl=sqrt(x[jjj].r*x[jjj].r + x[jjj].i*x[jjj].i)/max;

    if (b<0 || b>X*Y) {
      printz("fields error\n");
      stop(1);
    }
    if (1==1) {
      bitmapr[b]=255.99*fase*ampl;
      bitmapg[b]=255*ampl;
      bitmapb[b]=255.99*(1.0-fase)*ampl;
    }
    else if (0==1){
      bitmapr[b]=bitmapg[b]=bitmapb[b]=255.9*ampl;
    }
    else {
      bitmapr[b]=255*(x[jjj].r*x[jjj].r)/(max*max);
      bitmapb[b]=255*(x[jjj].i*x[jjj].i)/(max*max);
      bitmapg[b]=255*ampl;
    }
    if (position[3*i+0]==0) fprintz(fields,"%i %i %i %.4g %.4g %.4g %.4g %.4g %.4g\n",position[3*i],position[3*i+1],position[3*i+2],x[3*i].r,x[3*i].i,x[3*i+1].r,x[3*i+1].i,x[3*i+2].r,x[3*i+2].i);
    st1++;
  }
  
  /* save picture */
  for(i=0;i<X*Y;i++) fprintz(pbm,"%c%c%c",bitmapr[i],bitmapg[i],bitmapb[i]);
  fclose(pbm);
  fclose(fields);
}
